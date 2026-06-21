//***********************************
//* Source EnPhase Envoy V5 ou V7   *
//***********************************

uint32_t ipToInt(IPAddress ip) {                                                                  //SR19
  return uint32_t(ip[0] << 24) | uint32_t(ip[1] << 16) | uint32_t(ip[2] << 8) | uint32_t(ip[3]);  //SR19
}

void Setup_Enphase() {

  //Résolution mDNS de http://envoy.local en adresse IP                                                                  //SR19
  //***************************************************                                                                  //SR19

  const char* host = "envoy";  //SR19
  IPAddress envoyIP;
  if (RMSextIPauto) {                                                                                //SR19
    if (!MDNS.begin(hostname)) {                                                                     //Init mDNS                                                                              //SR19
      TelnetPrintln("Erreur : impossible d'initialiser mDNS");                                       //SR19
      return;                                                                                        //SR19
    } else {                                                                                         //SR19
      envoyIP = MDNS.queryHost(host, 2000);                                                          //avec timeout 2s                                                             //SR19
    }                                                                                                //SR19
    if (envoyIP.toString() != "0.0.0.0") {                                                           //SR19
      StockMessage("IP Enphase : http://" + String(host) + ".local" + " -> " + envoyIP.toString());  //SR19
      RMSextIP = ipToInt(envoyIP);                                                                   //IP -> uint32                                                                         //SR19
    } else {                                                                                         //SR19
      StockMessage("Échec! passerelle Enphase envoy déconnectée");                                   //SR19
      return;                                                   // SR19
    }
  }
  //Obtention Session ID
  //********************
  const char* server1Enphase = "enlighten.enphaseenergy.com";
  String Host = String(server1Enphase);
  String adrEnphase = "https://" + Host + "/login/login.json";
  String requestBody = "user[email]=" + EnphaseUser + "&user[password]=" + urlEncode(EnphasePwd);

  if (EnphaseUser != "" && EnphasePwd != "" && RMSextIP > 0) {  // test envoyIP si perte de connexion //SR19
    TelnetPrintln("Essai connexion  Enlighten server 1 pour obtention session_id!");
    clientSecu.setInsecure();  //skip verification
    if (!clientSecu.connect(server1Enphase, 443, 13000))
      StockMessage("Connection failed to Enlighten server :" + Host);
    else {
      TelnetPrintln("Connected to Enlighten server:" + Host);
      clientSecu.println("POST " + adrEnphase + "?" + requestBody + " HTTP/1.0");
      clientSecu.println("Host: " + Host);
      clientSecu.println("Connection: close");
      clientSecu.println();
      String line = "";
      while (clientSecu.connected()) {
        line = clientSecu.readStringUntil('\n');
        if (line == "\r") {
          TelnetPrintln("headers 1 Enlighten received");
          JsonToken = "";
        }

        JsonToken += line;
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (clientSecu.available()) {
        char c = clientSecu.read();
        Serial.write(c);
      }
      clientSecu.stop();
    }
    Session_id = StringJson("session_id", JsonToken);
    TelnetPrintln("session_id :" + Session_id);
  } else {
    TelnetPrintln("Connexion  vers Envoy-S en firmware version 5");
  }
  // Obtention Token
  //********************
  //  if (Session_id != "" && EnphaseSerial != "" && EnphaseUser != "") {
  //    const char* server2Enphase = "entrez.enphaseenergy.com";
  //    Host = String(server2Enphase);
  //    adrEnphase = "https://" + Host + "/tokens";
  //    requestBody = "{\"session_id\":\"" + Session_id + "\", \"serial_num\":" + EnphaseSerial + ", \"username\":\"" + EnphaseUser + "\"}";
  //    TelnetPrintln("Essai connexion  Enlighten server 2 pour obtention token!");
  //    clientSecu.setInsecure();  //skip verification
  //    if (!clientSecu.connect(server2Enphase, 443, 13000))
  //      StockMessage("Connection failed to :" + Host);
  //    else {
  //      TelnetPrintln("Connected to :" + Host);
  //      clientSecu.println("POST " + adrEnphase + " HTTP/1.0");
  //      clientSecu.println("Host: " + Host);
  //      clientSecu.println("Content-Type: application/json");
  //      clientSecu.println("content-length:" + String(requestBody.length()));
  //      clientSecu.println("Connection: close");
  //      clientSecu.println();
  //      clientSecu.println(requestBody);
  //      clientSecu.println();
  //      TelnetPrintln("Attente user est connecté");
  //      String line = "";
  //      JsonToken = "";
  //      while (clientSecu.connected()) {
  //        line = clientSecu.readStringUntil('\n');
  //        if (line == "\r") {
  //          TelnetPrintln("headers 2 enlighten received");
  //          JsonToken = "";
  //        }

  //       JsonToken += line;
  //     }
  //     // if there are incoming bytes available
  //     // from the server, read them and print them:
  //     while (clientSecu.available()) {
  //       char c = clientSecu.read();
  //       Serial.write(c);
  //     }
  //     clientSecu.stop();
  //     JsonToken.trim();
  //     TelnetPrintln("Token :" + JsonToken);
  //     if (JsonToken.length() > 50) {
  //       TokenEnphase = JsonToken;
  //       previousTimeRMSMin = 1000;
  //       previousTimeRMSMax = 1;
  //       previousTimeRMSMoy = 1;
  //       previousTimeRMS = millis();
  //       LastRMS_Millis = millis();
  //       PeriodeProgMillis = 1000;
  //     }
  //   }
  // }

}

void LectureEnphase()
{
  static long LastwhDlvdCum = 0L;
  static long LastwhRcvdCum = 0L;
  char host[16];
  snprintf(host, sizeof(host), "%lu.%lu.%lu.%lu", (RMSextIP >> 24) & 0xFF, (RMSextIP >> 16) & 0xFF, (RMSextIP >> 8) & 0xFF, RMSextIP & 0xFF);

  char baseRequest[50];
  snprintf(baseRequest, sizeof(baseRequest), "/ivp/meters/readings");

  // ============================================================
  // CORRECTION 1 : Initialisation explicite du filtre Envoy V7
  // ============================================================
  // Initialisé UNE SEULE FOIS au premier appel grace a la declaration en  static.
  static JsonDocument filter;
  // Si le filtre ne contient aucune clé, on l'initialise
  if (filter.size() == 0)
  {
    // En configurant l'index [0], ArduinoJson v6 va appliquer
    // ce masque à TOUS les objets du tableau principal.
    filter[0]["activePower"] = true;
    filter[0]["apparentPower"] = true;
    filter[0]["voltage"] = true;
    filter[0]["current"] = true;
    filter[0]["freq"] = true;
    filter[0]["actEnergyDlvd"] = true; // Sera extrait partout où il existe !
    filter[0]["actEnergyRcvd"] = true; // Sera extrait partout où il existe !
  }
  JsonDocument doc; // desalocation automatique de la mémoire à la fin de la fonction grace a arduinojson v7
  DeserializationError error = DeserializationError::EmptyInput;

  static uint32_t lastTokenUpdate = 0;
  constexpr uint32_t TOKEN_REFRESH_MS = 30UL * 24UL * 60UL * 60UL * 1000UL;

  if (lastTokenUpdate == 0)
    lastTokenUpdate = millis();

  if ((uint32_t)(millis() - lastTokenUpdate) > TOKEN_REFRESH_MS)
  {
    lastTokenUpdate = millis();
    Setup_Enphase();
  }

  static uint32_t lastCall = 0;
  if ((uint32_t)(millis() - lastCall) < 2000)
  {
    return; // min 2s entre requêtes
  }
  lastCall = millis();

  {
    NetworkClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);

    // StockMessage("Envoy HTTPS connect...");

    if (!client.connect(host, 443))
    {
      TelnetPrintln("TLS FAIL");
      return;
    }

    client.print(
        String("GET ") + baseRequest + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Accept: application/json\r\n" +
        "Authorization: Bearer " + TokenEnphase + "\r\n" +
        "Connection: close\r\n\r\n");

    String statusLine = client.readStringUntil('\n');
    statusLine.trim();
    TelnetPrintln("HTTP: " + statusLine);

    if (statusLine.indexOf("200") < 0)
    {
      TelnetPrintln("Envoy refused request");
      client.stop();
      return;
    }

    // On passe toutes les lignes de header restantes jusqu'à tomber sur la ligne vide (\r)
    while (client.connected())
    {
      String line = client.readStringUntil('\n');
      if (line == "\r" || line == "")
      {
        break; // Fin des headers atteinte, le JSON commence juste après
      }
    }

    // On attend un poil que les paquets TLS arrivent
    uint32_t startCheck = millis();
    while (client.available() == 0 && (millis() - startCheck) < 2000)
    {
      delay(10);
    }

    String jsonPayload;
    if (!jsonPayload.reserve(20000))
    {
      TelnetPrintln("reserve failed");
      return;
    }
    while (client.available() > 0 || client.connected())
    {
      if (client.available() > 0)
      {
        jsonPayload += (char)client.read();
      }
      else
      {
        delay(1); // Laisse respirer le stack TLS de l'ESP32
      }
    }

    client.stop(); // Fermeture propre immédiate du socket

    if (jsonPayload.length() == 0)
    {
      TelnetPrintln("JSON ERR: Payload vide");
      return;
    }
    else
    {
      TelnetPrintln("JSON payload length: " + String(jsonPayload.length()));
      TelnetPrintln("Free heap=" + String(ESP.getFreeHeap()));
    }

    error = deserializeJson(doc, static_cast<const String &>(jsonPayload), DeserializationOption::Filter(filter));

    if (error)
    {
      TelnetPrintln("JSON ERR: " + String(error.c_str()));
      return;
    }

    TelnetPrintln("JSON OK");
  }
  
  // --- LOGIQUE DE TRAITEMENT ET DISPATCH DES DONNÉES ---
  float PactReseau = 0.0f;
  float PvaReseau = 0.0f;
  long whDlvdCum = 0L;
  long whRcvdCum = 0L; // <--- AJOUT CRITIQUE POUR LE FIRMWARE V7

  if (!error)
  {
    PactProd = doc[0]["activePower"] | 0.0f;
    PactReseau = doc[1]["activePower"] | 0.0f;
    PactConso_M = PactReseau + PactProd;

    PvaReseau = doc[1]["apparentPower"] | 0.0f;
    // Extraction des index énergétiques du bloc réseau [1]
    whDlvdCum = (long)doc[1]["actEnergyDlvd"].as<double>();
    whRcvdCum = (long)doc[1]["actEnergyRcvd"].as<double>();

    Tension_M = doc[1]["voltage"] | 0.0f;
    Intensite_M = doc[1]["current"] | 0.0f;
    Frequence = doc[1]["freq"];
  }
  else
  {
    PactProd = 0.0f;
    PactConso_M = 0.0f;
    PactReseau = 0.0f;
    Tension_M = 0.0f;
    Intensite_M = 0.0f;
    Frequence = 0.0f;
  }

  // Calcul injection / soutirage inst.
  PactReseau = PfloatMax(PactReseau);
  // Calcul VA (Puissance apparente)
  // TRÈS IMPORTANT : Sur l'Envoy V7, apparentPower est TOUJOURS positif dans le JSON !
  PvaReseau = PfloatMax(PvaReseau);
  if (PactReseau < 0.0f)
  {
    PuissanceS_M_inst = 0;
    PuissanceI_M_inst = (int)(-PactReseau);
    PVAS_M_inst = 0;
    PVAI_M_inst = (int)(PvaReseau);
  }
  else
  {
    PuissanceI_M_inst = 0;
    PuissanceS_M_inst = (int)(PactReseau);
    PVAI_M_inst = 0;
    PVAS_M_inst = (int)(PvaReseau);
  }

  Pva_valide = true;
  filtre_puissance();

  // Facteur de puissance (Cos phi)
  float PowerFactor = 0.0f;
  if (PVA_M_moy != 0)
  {
    PowerFactor = floor(100.0f * fabsf(Puissance_M_moy) / (float)PVA_M_moy) / 100.0f;
    PowerFactor = min(PowerFactor, 1.0f);
  }
  PowerFactor_M = PowerFactor;

  if (whDlvdCum != 0)
  {
    if (LastwhDlvdCum == 0)
      LastwhDlvdCum = whDlvdCum;
    long DeltaWhSoutire = whDlvdCum - LastwhDlvdCum;
    LastwhDlvdCum = whDlvdCum;
    if (DeltaWhSoutire > 0)
    {
      Energie_M_Soutiree += DeltaWhSoutire;
    }
  }

  if (whRcvdCum != 0)
  {
    if (LastwhRcvdCum == 0)
      LastwhRcvdCum = whRcvdCum;
    long DeltaWhInjecte = whRcvdCum - LastwhRcvdCum;
    LastwhRcvdCum = whRcvdCum;
    if (DeltaWhInjecte > 0)
    {
      Energie_M_Injectee += DeltaWhInjecte;
    }
  }
  TelnetPrintln(
      "Prod=" + String(PactProd) +
      " Grid=" + String(PactReseau) +
      " Conso=" + String(PactConso_M));
  // Validation des flags et reset des surveillances
  EnergieActiveValide = true;
  if (PactReseau != 0.0f || PvaReseau != 0.0f)
    PuissanceRecue = true;
  if (cptLEDyellow > 30)
    cptLEDyellow = 4;
}

String PrefiltreJson(String F1, String F2, String Json)
{
  int p = Json.indexOf(F1);
  Json = Json.substring(p);
  p = Json.indexOf(F2);
  Json = Json.substring(p);
  return Json;
}
String SubJson(String F1, String F2, String Json)
{
  int p = Json.indexOf(F1);
  Json = Json.substring(p);
  p = Json.indexOf(F2);
  Json = Json.substring(0, p + 1);
  return Json;
}

float ValJson(String nom, String Json)
{
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  p = Json.indexOf("}");
  p = min(p, q);
  float val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toFloat();
  }
  return val;
}
long LongJson(String nom, String Json)
{ // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(".");
  p = Json.indexOf("}");
  p = min(p, q);
  long val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}

long myLongJson(String nom, String Json)
{ // Alternative a LongJson au dessus pour extraire chez RTE nb jour Tempo  https://particulier.RTE.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(","); //<==== Recherche d'une virgule et non d'un point
  if (q == -1)
    q = 999; //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  p = min(p, q);
  long val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned long ULongJson(String nom, String Json)
{ // Alternative a LongJson au dessus pour extraire chez RTE nb jour Tempo  https://particulier.RTE.fr/services/rest/referentiel/getNbTempoDays?TypeAlerte=TEMPO
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(","); //<==== Recherche d'une virgule et non d'un point
  if (q == -1)
    q = 999; //  /<==== Ajout de ces 2 lignes pour que la ligne p = min(p, q); ci dessous donne le bon résultat
  p = Json.indexOf("}");
  p = min(p, q);
  unsigned long val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    Json = "0000" + Json;
    int L = Json.length();
    unsigned long y = (Json.substring(0, L - 5)).toInt(); // Problème des valeurs signées dans un unsigned
    unsigned long z = (Json.substring(L - 5)).toInt();
    val = (y * 100000) + z;
  }
  return val;
}
int IntJson(String nom, String Json)
{ // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1)
    q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  int val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
byte ByteJson(String nom, String Json)
{ // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1)
    q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  byte val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
unsigned short UShortJson(String nom, String Json)
{ // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1)
    q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  unsigned short val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}
short ShortJson(String nom, String Json)
{ // Pour éviter des problèmes d'overflow
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  int q = Json.indexOf(",");
  if (q == -1)
    q = 999;
  p = Json.indexOf("}");
  p = min(p, q);
  short val = 0;
  if (p > 0)
  {
    Json = Json.substring(0, p);
    val = Json.toInt();
  }
  return val;
}

String StringJson(String nom, String Json)
{
  int p = Json.indexOf(nom + "\":");
  Json = Json.substring(p);
  p = Json.indexOf(":");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(p + 1);
  p = Json.indexOf("\"");
  Json = Json.substring(0, p);
  return Json;
}
