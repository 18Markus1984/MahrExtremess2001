#include <Arduino.h>

//Raw content of index.html in plain text
static const char *index_html = R"--espform--(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>ESP32 Messgerät Steuerung</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f0f2f5;
      margin: 0;
      padding: 1rem;
    }

    .container {
      max-width: 800px;
      margin: 0 auto;
    }

    header {
      background-color: #0056b3;
      color: white;
      padding: 1rem;
      font-size: 1.5rem;
      text-align: center;
    }


    .block {
      background-color: #fff;
      padding: 1.5rem;
      margin-bottom: 2rem;
      border-radius: 10px;
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
    }

    h2 {
      color: #003366;
      margin-bottom: 1rem;
    }

    label {
      display: block;
      margin-top: 1rem;
      font-weight: bold;
    }

    input, select, button {
      width: 100%;
      padding: 0.5rem;
      margin-top: 0.3rem;
      border: 1px solid #ccc;
      border-radius: 5px;
    }

    button {
      background-color: #007bff;
      color: #fff;
      border: none;
      margin-top: 1rem;
      cursor: pointer;
      font-weight: bold;
    }

    button:hover {
      background-color: #0056b3;
    }

    .macro-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
      gap: 0.5rem;
      margin-top: 1rem;
    }

    ul {
      padding-left: 1.2rem;
    }

    footer {
      text-align: center;
      font-size: 0.9rem;
      color: #777;
      margin-top: 3rem;
    }

  </style>
</head>

<body>

  <header>Messmodus auswählen</header>

  <div class="container">
    <section class="block">
  <h2>Live-Datenvisualisierung</h2>
  <p>Der folgende Graph zeigt die aktuellen Messwerte in Echtzeit an. Nutzen Sie die Zoom-Funktion, um Details zu analysieren.</p>

  <div id="loading" style="text-align:center;">Lade Daten...</div>
  
  <div id="container" style="display:none; margin-top: 1rem;">
    <div id="chart-container" style="height: 400px; width: 100%;"></div>
  </div>

  <div style="text-align:center; margin-top: 1rem;">
    <button type="button" id="clear-btn" class="button" style="display:none;">Zurücksetzen</button>
  </div>
  </section>

<!-- Stelle sicher, dass Highcharts und main.js eingebunden sind -->
  <script src="highcharts.js"></script>
  <script src="main.js"></script>


    <!-- 🔁 MODUS BLOCK -->
    <section class="block">
      <h2>Moduswahl</h2>
      <form>
        <label for="modeSelect">Messmodus:</label>
        <select id="modeSelectComBox" onchange="toggleSettings()">
          <option value="repeatability">Wiederholgenauigkeit</option>
          <option value="overshoot">Überschwingverhalten</option>
        </select>

        <div id="repeatabilitySettings">
          <label for="averageCount">Anzahl Messwerte für Mittelwert:</label>
          <input type="number" id="averageCount" min="1" max="100" value="40" />

          <label for="tolerance">Toleranzbereich (mm):</label>
          <input type="number" id="tolerance" step="0.001" value="0.005" />
        </div>

        <button type="button" id="startStopBtn" onclick="toggleMeasurement()">Start</button>
      </form>
    </section>

    <!-- ℹ️ INFOS BLOCK -->
    <section class="block">
      <h2>Allgemeine Informationen</h2>
      <p>Dieses Interface steuert das Mahr Extramess 2001 über einen ESP32-C3 und ermöglicht die direkte Interaktion mit allen wichtigen Gerätefunktionen.</p>

      <p>Verfügbare Messmodi:</p>
      <ul>
        <li><strong>Wiederholgenauigkeit:</strong> Berechnet einen stabilen Mittelwert anhand mehrerer Messwerte. Überprüft, ob er innerhalb einer vorgegebenen Toleranz liegt.</li>
        <li><strong>Überschwingverhalten:</strong> Visualisiert das zeitliche Verhalten bei Bewegungen.</li>
      </ul>

      <p>Makro-Befehle zur Steuerung des Geräts:</p>
      <ul>
        <li><strong>Range 1 / 2 / 3 (RES1 / RES2 / RES3):</strong> Wechselt zwischen den drei voreingestellten Messbereichen.</li>
        <li><strong>Toleranz? (TOL?):</strong> Fragt die aktuell konfigurierten Toleranzen ab.</li>
        <li><strong>Status? (SET?):</strong> Zeigt den aktuellen Gerätestatus an.</li>
        <li><strong>Messwert (?)</strong> Fordert den aktuellen Messwert an.</li>
        <li><strong>Reset (RST):</strong> Setzt das Gerät zurück und deaktiviert den ABS-Modus.</li>
        <li><strong>Batterie? (BAT?):</strong> Fragt den Batteriestatus ab.</li>
        <li><strong>ABS (ABS):</strong> Aktiviert den absoluten Messmodus.</li>
        <li><strong>Max / Min (MAX / MIN):</strong> Zeigt den maximalen oder minimalen Messwert.</li>
        <li><strong>Ausschalten (OFF):</strong> Versetzt das Gerät in den Ruhemodus / schaltet es aus.</li>
      </ul>

      <p>Die effektive Abtastrate liegt bei ca. 14,5 Hz. Das System verwendet das serielle 7E2-Protokoll über UART (4800 Baud).</p>
    </section>


    <!-- ⚙️ MAKRO BLOCK -->
    <section class="block">
      <h2>Makros</h2>
      <p>Direkte Befehle zur Kommunikation mit dem Messgerät:</p>
      <div class="macro-grid">
        <button onclick="sendMacro('RES1\\r')">Range 1</button>
        <button onclick="sendMacro('RES2\\r')">Range 2</button>
        <button onclick="sendMacro('RES3\\r')">Range 3</button>
        <button onclick="sendMacro('TOL?\\r')">Toleranz?</button>
        <button onclick="sendMacro('SET?\\r')">Status?</button>
        <button onclick="sendMacro('?\\r')">Messwert</button>
        <button onclick="sendMacro('RST\\r')">Reset</button>
        <button onclick="sendMacro('BAT?\\r')">Batterie?</button>
        <button onclick="sendMacro('ABS\\r')">ABS</button>
        <button onclick="sendMacro('MAX\\r')">Max</button>
        <button onclick="sendMacro('MIN\\r')">Min</button>
        <button onclick="sendMacro('OFF\\r')">Ausschalten</button>
      </div>
    </section>

  </div>
  <footer>© 2025 ESP32 Messinterface</footer>
  <script>
    function toggleMeasurement() {
      const btn = document.getElementById('startStopBtn');
      if (btn.textContent === "Start") {
        btn.textContent = "Stopp";
        btn.style.backgroundColor = "#dc3545";
      } else {
        btn.textContent = "Start";
        btn.style.backgroundColor = "#007bff";
      }
    }

    function toggleSettings() {
      const settings = document.getElementById('repeatabilitySettings');
      const mode = document.getElementById('modeSelect').value;
      settings.style.display = mode === "repeatability" ? "block" : "none";
    }

    function sendMacro(command) {
      console.log("Sende Befehl: ", command);
      // Optional: hier würde ein fetch oder WebSerial-Befehl kommen
    }

    // Init
    toggleSettings();
  </script>
</body>
</html>
)--espform--";
