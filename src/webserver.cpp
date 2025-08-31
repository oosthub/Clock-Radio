#include "webserver.h"
#include "settings.h"
#include "weather.h"
#include "wifi_config.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

AsyncWebServer server(80);

// Stream storage for web interface
WebRadioStream webStreams[MAX_WEB_STREAMS];
int webStreamCount = 0;

// HTML page for managing streams
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>OOSIE Radio - Stream Manager</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background-color: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; margin-bottom: 10px; }
        h2 { color: #666; text-align: center; margin-top: 0; margin-bottom: 30px; font-weight: normal; }
        
        /* Tab Navigation Styles */
        .tab-container { margin-bottom: 20px; }
        .tab-nav { display: flex; border-bottom: 2px solid #4CAF50; margin-bottom: 20px; }
        .tab-nav button { 
            background: none; 
            border: none; 
            padding: 12px 24px; 
            cursor: pointer; 
            font-size: 16px; 
            color: #666; 
            border-bottom: 3px solid transparent;
            transition: all 0.3s ease;
        }
        .tab-nav button:hover { background-color: #f5f5f5; color: #333; }
        .tab-nav button.active { 
            color: #4CAF50; 
            border-bottom-color: #4CAF50; 
            font-weight: bold; 
        }
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        
        table { width: 100%; border-collapse: collapse; margin-bottom: 20px; }
        th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background-color: #4CAF50; color: white; }
        tr:hover { background-color: #f5f5f5; }
        input[type="text"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }
        .name-input { max-width: 150px; }
        .url-input { min-width: 300px; }
        button { background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer; margin: 2px; }
        button:hover { background-color: #45a049; }
        .delete-btn { background-color: #f44336; }
        .delete-btn:hover { background-color: #da190b; }
        .add-row { background-color: #e8f5e8; }
        .status { padding: 10px; margin: 10px 0; border-radius: 4px; }
        .success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .char-count { font-size: 12px; color: #666; }
        .over-limit { color: #f44336; font-weight: bold; }
        
        /* Manual Styles */
        .manual-content { 
            line-height: 1.6; 
            max-height: 600px; 
            overflow-y: auto; 
            border: 1px solid #ddd; 
            padding: 20px; 
            background-color: #fafafa; 
            border-radius: 5px; 
        }
        .manual-content h1, .manual-content h2, .manual-content h3 { 
            color: #333; 
            margin-top: 20px; 
            margin-bottom: 10px; 
        }
        .manual-content code { 
            background-color: #f1f1f1; 
            padding: 2px 4px; 
            border-radius: 3px; 
            font-family: monospace; 
        }
        .manual-content pre { 
            background-color: #f1f1f1; 
            padding: 10px; 
            border-radius: 5px; 
            overflow-x: auto; 
        }
        .loading { text-align: center; padding: 40px; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎵 OOSIE Radio</h1>
        <h2>Internet Radio - Management Interface</h2>
        <div id="status"></div>
        
        <!-- Tab Navigation -->
        <div class="tab-container">
            <div class="tab-nav">
                <button class="tab-button active" onclick="openTab(event, 'streams-tab')">📻 Stream Manager</button>
                <button class="tab-button" onclick="openTab(event, 'weather-tab')">🌤️ Weather Settings</button>
                <button class="tab-button" onclick="openTab(event, 'wifi-tab')">📶 WiFi Settings</button>
                <button class="tab-button" onclick="openTab(event, 'manual-tab')">📖 User Manual</button>
            </div>
        </div>
        
        <!-- Streams Tab -->
        <div id="streams-tab" class="tab-content active">
        <table id="streamTable">
            <thead>
                <tr>
                    <th>Name (max 16 chars)</th>
                    <th>URL</th>
                    <th>Actions</th>
                </tr>
            </thead>
            <tbody id="streamTableBody">
            </tbody>
            <tfoot>
                <tr class="add-row">
                    <td>
                        <input type="text" id="newName" placeholder="Stream name" maxlength="16">
                        <div class="char-count" id="nameCharCount">0/16</div>
                    </td>
                    <td>
                        <input type="text" id="newUrl" placeholder="http://example.com/stream.m3u8" class="url-input">
                    </td>
                    <td>
                        <button onclick="addStream()">Add Stream</button>
                    </td>
                </tr>
            </tfoot>
        </table>
        
        <div style="text-align: center; margin-top: 30px;">
            <button onclick="saveStreams()" style="font-size: 16px; padding: 12px 24px;">💾 Save All Changes</button>
        </div>
        </div>
        
        <!-- WiFi Settings Tab -->
        <div id="wifi-tab" class="tab-content">
            <h3>📶 WiFi Configuration</h3>
            <p>Configure WiFi network connection for your radio.</p>
            <div style="margin-bottom: 15px;">
                <label for="wifiSSID" style="display: block; margin-bottom: 5px; font-weight: bold;">WiFi Network (SSID):</label>
                <input type="text" id="wifiSSID" placeholder="Enter WiFi network name" style="width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px;">
            </div>
            <div style="margin-bottom: 15px;">
                <label for="wifiPassword" style="display: block; margin-bottom: 5px; font-weight: bold;">WiFi Password:</label>
                <input type="password" id="wifiPassword" placeholder="Enter WiFi password" style="width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px;">
                <div style="margin-top: 5px;">
                    <input type="checkbox" id="showPassword" onchange="togglePasswordVisibility()">
                    <label for="showPassword" style="font-size: 14px; color: #666;">Show password</label>
                </div>
            </div>
            <div style="text-align: center;">
                <button onclick="saveWiFiSettings()" style="background-color: #28a745; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; margin-right: 10px;">💾 Save & Restart</button>
                <button onclick="testWiFiConnection()" style="background-color: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer;">🔍 Test Connection</button>
            </div>
            <div id="wifiStatus" style="margin-top: 15px;"></div>
        </div>
        
        <!-- Weather Tab -->
        <div id="weather-tab" class="tab-content">
            <h3>🌤️ Weather Configuration</h3>
            <p>Get your free API key from <a href="https://openweathermap.org/api" target="_blank">OpenWeatherMap</a></p>
            <div style="margin-bottom: 15px;">
                <label for="weatherApiKey" style="display: block; margin-bottom: 5px; font-weight: bold;">Weather API Key:</label>
                <input type="text" id="weatherApiKey" placeholder="Enter your OpenWeatherMap API key" style="width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px;">
            </div>
            <div style="text-align: center;">
                <button onclick="saveWeatherSettings()" style="background-color: #17a2b8; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer;">💾 Save Weather Settings</button>
            </div>
        </div>
        
        <!-- User Manual Tab -->
        <div id="manual-tab" class="tab-content">
            <h3>📖 User Manual</h3>
            <div style="text-align: right; margin-bottom: 10px;">
                <button onclick="loadUserManual()" style="background-color: #6c757d; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer;">🔄 Refresh Manual</button>
            </div>
            <div id="manualContent" class="manual-content">
                <div class="loading">Loading user manual...</div>
            </div>
        </div>
    </div>

    <script>
        let streams = [];

        // Tab functionality
        function openTab(evt, tabName) {
            var i, tabcontent, tabbuttons;
            
            // Hide all tab contents
            tabcontent = document.getElementsByClassName("tab-content");
            for (i = 0; i < tabcontent.length; i++) {
                tabcontent[i].classList.remove("active");
            }
            
            // Remove active class from all tab buttons
            tabbuttons = document.getElementsByClassName("tab-button");
            for (i = 0; i < tabbuttons.length; i++) {
                tabbuttons[i].classList.remove("active");
            }
            
            // Show the selected tab content and mark button as active
            document.getElementById(tabName).classList.add("active");
            evt.currentTarget.classList.add("active");
        }

        // Load streams on page load
        window.onload = function() {
            loadStreams();
            loadWeatherSettings();
            loadWiFiSettings();
            setupCharCounters();
            loadUserManual();
        };

        function setupCharCounters() {
            document.getElementById('newName').addEventListener('input', function() {
                updateCharCount('newName', 'nameCharCount');
            });
        }

        function updateCharCount(inputId, countId) {
            const input = document.getElementById(inputId);
            const counter = document.getElementById(countId);
            const count = input.value.length;
            counter.textContent = count + '/16';
            
            if (count > 16) {
                counter.classList.add('over-limit');
                input.style.borderColor = '#f44336';
            } else {
                counter.classList.remove('over-limit');
                input.style.borderColor = '#ddd';
            }
        }

        function loadStreams() {
            fetch('/get-streams')
                .then(response => response.json())
                .then(data => {
                    streams = data;
                    updateTable();
                })
                .catch(error => {
                    showStatus('Error loading streams: ' + error, 'error');
                });
        }

        function updateTable() {
            const tbody = document.getElementById('streamTableBody');
            tbody.innerHTML = '';
            
            streams.forEach((stream, index) => {
                const row = tbody.insertRow();
                row.innerHTML = `
                    <td>
                        <input type="text" value="${stream.name}" onchange="updateStream(${index}, 'name', this.value)" maxlength="16">
                        <div class="char-count">${stream.name.length}/16</div>
                    </td>
                    <td>
                        <input type="text" value="${stream.url}" onchange="updateStream(${index}, 'url', this.value)" class="url-input">
                    </td>
                    <td>
                        <button class="delete-btn" onclick="deleteStream(${index})">Delete</button>
                    </td>
                `;
            });
        }

        function updateStream(index, field, value) {
            if (field === 'name' && value.length > 16) {
                showStatus('Stream name cannot exceed 16 characters', 'error');
                return;
            }
            streams[index][field] = value;
        }

        function addStream() {
            const name = document.getElementById('newName').value.trim();
            const url = document.getElementById('newUrl').value.trim();
            
            if (!name || !url) {
                showStatus('Please enter both name and URL', 'error');
                return;
            }
            
            if (name.length > 16) {
                showStatus('Stream name cannot exceed 16 characters', 'error');
                return;
            }
            
            if (!url.startsWith('http://') && !url.startsWith('https://')) {
                showStatus('URL must start with http:// or https://', 'error');
                return;
            }
            
            streams.push({ name: name, url: url });
            updateTable();
            
            // Clear inputs
            document.getElementById('newName').value = '';
            document.getElementById('newUrl').value = '';
            document.getElementById('nameCharCount').textContent = '0/16';
            
            showStatus('Stream added. Remember to save changes!', 'success');
        }

        function deleteStream(index) {
            if (confirm('Are you sure you want to delete this stream?')) {
                streams.splice(index, 1);
                updateTable();
                showStatus('Stream deleted. Remember to save changes!', 'success');
            }
        }

        function saveStreams() {
            if (streams.length === 0) {
                showStatus('Cannot save empty stream list', 'error');
                return;
            }
            
            // Validate all streams
            for (let i = 0; i < streams.length; i++) {
                if (!streams[i].name || !streams[i].url) {
                    showStatus('All streams must have both name and URL', 'error');
                    return;
                }
                if (streams[i].name.length > 16) {
                    showStatus('All stream names must be 16 characters or less', 'error');
                    return;
                }
            }
            
            fetch('/update-streams', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(streams)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showStatus('Streams saved successfully! Radio will restart...', 'success');
                } else {
                    showStatus('Error: ' + data.message, 'error');
                }
            })
            .catch(error => {
                showStatus('Error saving streams: ' + error, 'error');
            });
        }

        function showStatus(message, type) {
            const statusDiv = document.getElementById('status');
            statusDiv.innerHTML = '<div class="status ' + type + '">' + message + '</div>';
            setTimeout(() => {
                statusDiv.innerHTML = '';
            }, 5000);
        }

        function loadWeatherSettings() {
            fetch('/get-weather-settings')
                .then(response => response.json())
                .then(data => {
                    const apiKeyInput = document.getElementById('weatherApiKey');
                    apiKeyInput.value = data.apiKey || '';
                    
                    // Store if this is a masked value
                    apiKeyInput.dataset.isMasked = data.isConfigured ? 'true' : 'false';
                    
                    // Add placeholder text if masked
                    if (data.isConfigured && data.apiKey.includes('*')) {
                        apiKeyInput.placeholder = 'API key is configured (masked for security)';
                    }
                })
                .catch(error => {
                    console.log('Weather settings not available:', error);
                });
        }

        function saveWeatherSettings() {
            const apiKeyInput = document.getElementById('weatherApiKey');
            const apiKey = apiKeyInput.value.trim();
            
            // If the field contains masked data and user hasn't changed it, don't save
            if (apiKeyInput.dataset.isMasked === 'true' && apiKey.includes('*')) {
                showStatus('API key is already configured. Enter a new key to change it.', 'error');
                return;
            }
            
            if (!apiKey) {
                showStatus('Please enter a weather API key', 'error');
                return;
            }
            
            fetch('/update-weather-settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ apiKey: apiKey })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showStatus('Weather settings saved successfully!', 'success');
                    // Reload the masked value after saving
                    setTimeout(() => {
                        loadWeatherSettings();
                    }, 1000);
                } else {
                    showStatus('Error: ' + data.message, 'error');
                }
            })
            .catch(error => {
                showStatus('Error saving weather settings: ' + error, 'error');
            });
        }

        function togglePasswordVisibility() {
            const passwordInput = document.getElementById('wifiPassword');
            const showPasswordCheckbox = document.getElementById('showPassword');
            
            if (showPasswordCheckbox.checked) {
                passwordInput.type = 'text';
            } else {
                passwordInput.type = 'password';
            }
        }

        function testWiFiConnection() {
            const ssid = document.getElementById('wifiSSID').value.trim();
            const password = document.getElementById('wifiPassword').value;
            
            if (!ssid) {
                showWiFiStatus('Please enter a WiFi network name (SSID)', 'error');
                return;
            }
            
            showWiFiStatus('Testing connection...', 'info');
            
            fetch('/test-wifi-connection', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ ssid: ssid, password: password })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showWiFiStatus('Connection successful! IP: ' + data.ip, 'success');
                } else {
                    showWiFiStatus('Connection failed: ' + data.message, 'error');
                }
            })
            .catch(error => {
                showWiFiStatus('Error testing connection: ' + error, 'error');
            });
        }

        function saveWiFiSettings() {
            const ssid = document.getElementById('wifiSSID').value.trim();
            const password = document.getElementById('wifiPassword').value;
            
            if (!ssid) {
                showWiFiStatus('Please enter a WiFi network name (SSID)', 'error');
                return;
            }
            
            if (confirm('Save WiFi settings and restart the radio?\\n\\nSSID: ' + ssid)) {
                showWiFiStatus('Saving WiFi settings...', 'info');
                
                fetch('/update-wifi-settings', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({ ssid: ssid, password: password })
                })
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        showWiFiStatus('WiFi settings saved! Radio is restarting...', 'success');
                        setTimeout(() => {
                            showWiFiStatus('Please wait 30 seconds, then connect to your regular WiFi network and access the radio at its new IP address.', 'info');
                        }, 3000);
                    } else {
                        showWiFiStatus('Error: ' + data.message, 'error');
                    }
                })
                .catch(error => {
                    showWiFiStatus('Error saving WiFi settings: ' + error, 'error');
                });
            }
        }

        function showWiFiStatus(message, type) {
            const statusDiv = document.getElementById('wifiStatus');
            const className = type === 'success' ? 'success' : type === 'error' ? 'error' : 'status';
            statusDiv.innerHTML = '<div class="status ' + className + '">' + message + '</div>';
            
            if (type !== 'info') {
                setTimeout(() => {
                    statusDiv.innerHTML = '';
                }, 10000);
            }
        }

        function loadUserManual() {
            const manualContent = document.getElementById('manualContent');
            manualContent.innerHTML = '<div class="loading">Loading user manual...</div>';
            
            // Fetch the user manual from GitHub
            const githubUrl = 'https://raw.githubusercontent.com/oosthub/Clock-Radio/main/USER_MANUAL.md';
            
            fetch(githubUrl)
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Failed to load user manual from GitHub');
                    }
                    return response.text();
                })
                .then(markdown => {
                    // Simple markdown to HTML conversion
                    const html = convertMarkdownToHtml(markdown);
                    manualContent.innerHTML = html;
                })
                .catch(error => {
                    manualContent.innerHTML = `
                        <div style="color: #721c24; background-color: #f8d7da; padding: 15px; border-radius: 5px; border: 1px solid #f5c6cb;">
                            <strong>Error loading user manual:</strong><br>
                            ${error.message}<br><br>
                            <small>Make sure the device has internet access and the GitHub repository is public.</small>
                        </div>
                    `;
                });
        }

        function convertMarkdownToHtml(markdown) {
            let html = markdown;
            
            // Convert headers
            html = html.replace(/^### (.*$)/gim, '<h3>$1</h3>');
            html = html.replace(/^## (.*$)/gim, '<h2>$1</h2>');
            html = html.replace(/^# (.*$)/gim, '<h1>$1</h1>');
            
            // Convert bold text
            html = html.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>');
            
            // Convert italic text
            html = html.replace(/\*(.*?)\*/g, '<em>$1</em>');
            
            // Convert code blocks
            html = html.replace(/```([^`]*)```/g, '<pre>$1</pre>');
            
            // Convert inline code
            html = html.replace(/`([^`]*)`/g, '<code>$1</code>');
            
            // Convert line breaks
            html = html.replace(/\n/g, '<br>');
            
            // Convert lists (simple version)
            html = html.replace(/^\- (.*$)/gim, '<li>$1</li>');
            html = html.replace(/(<li>.*<\/li>)/s, '<ul>$1</ul>');
            
            return html;
        }

        function loadWiFiSettings() {
            fetch('/get-wifi-settings')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('wifiSSID').value = data.ssid || '';
                    // Don't load password for security
                    document.getElementById('wifiPassword').placeholder = data.hasPassword ? 
                        'Password configured (leave blank to keep current)' : 'Enter WiFi password';
                })
                .catch(error => {
                    console.log('WiFi settings not available:', error);
                });
        }

        function togglePasswordVisibility() {
            const passwordField = document.getElementById('wifiPassword');
            const checkbox = document.getElementById('showPassword');
            passwordField.type = checkbox.checked ? 'text' : 'password';
        }

        function saveWiFiSettings() {
            const ssid = document.getElementById('wifiSSID').value.trim();
            const password = document.getElementById('wifiPassword').value;
            
            if (!ssid) {
                showStatus('Please enter a WiFi network name (SSID)', 'error');
                return;
            }
            
            if (!password && !document.getElementById('wifiPassword').placeholder.includes('configured')) {
                showStatus('Please enter a WiFi password', 'error');
                return;
            }
            
            const wifiStatus = document.getElementById('wifiStatus');
            wifiStatus.innerHTML = '<div style="color: #856404; background-color: #fff3cd; padding: 10px; border-radius: 5px; border: 1px solid #ffeaa7;">💾 Saving WiFi settings and restarting radio...</div>';
            
            fetch('/update-wifi-settings', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ 
                    ssid: ssid, 
                    password: password 
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    wifiStatus.innerHTML = '<div style="color: #155724; background-color: #d4edda; padding: 10px; border-radius: 5px; border: 1px solid #c3e6cb;">✅ WiFi settings saved! Radio is restarting and will connect to the new network...</div>';
                } else {
                    wifiStatus.innerHTML = '<div style="color: #721c24; background-color: #f8d7da; padding: 10px; border-radius: 5px; border: 1px solid #f5c6cb;">❌ Error: ' + data.message + '</div>';
                }
            })
            .catch(error => {
                wifiStatus.innerHTML = '<div style="color: #721c24; background-color: #f8d7da; padding: 10px; border-radius: 5px; border: 1px solid #f5c6cb;">❌ Error saving WiFi settings: ' + error + '</div>';
            });
        }

        function testWiFiConnection() {
            const ssid = document.getElementById('wifiSSID').value.trim();
            const password = document.getElementById('wifiPassword').value;
            
            if (!ssid) {
                showStatus('Please enter a WiFi network name to test', 'error');
                return;
            }
            
            const wifiStatus = document.getElementById('wifiStatus');
            wifiStatus.innerHTML = '<div style="color: #856404; background-color: #fff3cd; padding: 10px; border-radius: 5px; border: 1px solid #ffeaa7;">🔍 Testing WiFi connection...</div>';
            
            fetch('/test-wifi-connection', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ 
                    ssid: ssid, 
                    password: password 
                })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    wifiStatus.innerHTML = '<div style="color: #155724; background-color: #d4edda; padding: 10px; border-radius: 5px; border: 1px solid #c3e6cb;">✅ Connection test successful! You can now save these settings.</div>';
                } else {
                    wifiStatus.innerHTML = '<div style="color: #721c24; background-color: #f8d7da; padding: 10px; border-radius: 5px; border: 1px solid #f5c6cb;">❌ Connection failed: ' + data.message + '</div>';
                }
            })
            .catch(error => {
                wifiStatus.innerHTML = '<div style="color: #721c24; background-color: #f8d7da; padding: 10px; border-radius: 5px; border: 1px solid #f5c6cb;">❌ Error testing connection: ' + error + '</div>';
            });
        }
    </script>
</body>
</html>
)rawliteral";

void initWebServer() {
    // Load streams from file
    loadStreamsFromFile();
    
    // Setup web server routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html; charset=UTF-8", htmlPage);
    });
    
    server.on("/get-streams", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(2048);
        JsonArray array = doc.to<JsonArray>();
        
        for (int i = 0; i < webStreamCount; i++) {
            JsonObject stream = array.createNestedObject();
            stream["name"] = webStreams[i].name;
            stream["url"] = webStreams[i].url;
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/update-streams", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Empty handler - actual processing in body handler
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        // Save to file
        File file = SPIFFS.open("/streams.json", "w");
        if (file) {
            serializeJson(doc, file);
            file.close();
            
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Streams updated successfully\"}");
            
            // Restart ESP32 to reload streams
            delay(1000);
            ESP.restart();
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to save streams\"}");
        }
    });
    
    // Weather settings endpoints
    server.on("/get-weather-settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        // Mask the API key if it exists (show first 3 and last 3 chars)
        String maskedApiKey = "";
        if (weatherApiKey.length() > 6) {
            maskedApiKey = weatherApiKey.substring(0, 3);
            for (int i = 0; i < (int)(weatherApiKey.length() - 6); i++) {
                maskedApiKey += "*";
            }
            maskedApiKey += weatherApiKey.substring(weatherApiKey.length() - 3);
        } else if (weatherApiKey.length() > 0) {
            for (int i = 0; i < (int)weatherApiKey.length(); i++) {
                maskedApiKey += "*";
            }
        }
        
        doc["apiKey"] = maskedApiKey;
        doc["isConfigured"] = weatherApiKey.length() > 0;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/update-weather-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Empty handler - actual processing in body handler
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        const char* apiKey = doc["apiKey"];
        if (apiKey) {
            weatherApiKey = String(apiKey);
            saveSettings(); // Save to EEPROM
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Weather settings saved\"}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing API key\"}");
        }
    });
    
    // WiFi settings endpoints
    server.on("/get-wifi-settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        doc["ssid"] = ssid;
        doc["hasPassword"] = password.length() > 0;
        // Never send the actual password for security
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    server.on("/update-wifi-settings", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Empty handler - actual processing in body handler
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        const char* newSSID = doc["ssid"];
        const char* newPassword = doc["password"];
        
        if (!newSSID || strlen(newSSID) == 0) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"SSID is required\"}");
            return;
        }
        
        // Update WiFi credentials
        ssid = String(newSSID);
        if (newPassword && strlen(newPassword) > 0) {
            password = String(newPassword);
        }
        // If password is empty/null, keep existing password
        
        saveSettings(); // Save to EEPROM
        request->send(200, "application/json", "{\"success\":true,\"message\":\"WiFi settings saved\"}");
        
        // Restart ESP32 to connect with new credentials
        delay(1000);
        ESP.restart();
    });
    
    server.on("/test-wifi-connection", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Empty handler - actual processing in body handler
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            return;
        }
        
        const char* testSSID = doc["ssid"];
        const char* testPassword = doc["password"];
        
        if (!testSSID || strlen(testSSID) == 0) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"SSID is required\"}");
            return;
        }
        
        // Test connection without saving
        WiFi.disconnect();
        delay(500);
        
        WiFi.begin(testSSID, testPassword ? testPassword : "");
        
        // Wait up to 15 seconds for connection
        int attempts = 30; // 30 * 500ms = 15 seconds
        while (WiFi.status() != WL_CONNECTED && attempts > 0) {
            delay(500);
            attempts--;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            String ip = WiFi.localIP().toString();
            WiFi.disconnect(); // Disconnect test connection
            delay(500);
            
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Connection successful! IP: " + ip + "\"}");
        } else {
            request->send(200, "application/json", "{\"success\":false,\"message\":\"Failed to connect - check SSID and password\"}");
        }
    });
    
    server.begin();
    Serial.println("Web server started");
    Serial.print("Open http://");
    Serial.print(WiFi.localIP());
    Serial.println(" in your browser to manage streams");
}

void handleWebServer() {
    // AsyncWebServer handles requests automatically, no need to call handleClient()
}

void loadStreamsFromFile() {
    File file = SPIFFS.open("/streams.json", "r");
    if (!file) {
        Serial.println("No streams.json file found, using default streams");
        saveStreamsToFile(); // Create default file
        return;
    }
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.println("Failed to parse streams.json, using defaults");
        saveStreamsToFile();
        return;
    }
    
    webStreamCount = 0;
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject stream : array) {
        if (webStreamCount >= MAX_WEB_STREAMS) break;
        
        const char* name = stream["name"];
        const char* url = stream["url"];
        
        if (name && url) {
            strncpy(webStreams[webStreamCount].name, name, 16);
            webStreams[webStreamCount].name[16] = '\0'; // Ensure null termination
            strncpy(webStreams[webStreamCount].url, url, 255);
            webStreams[webStreamCount].url[255] = '\0'; // Ensure null termination
            webStreamCount++;
        }
    }
    
    Serial.println("Streams loaded from JSON file");
}

void saveStreamsToFile() {
    DynamicJsonDocument doc(2048);
    JsonArray array = doc.to<JsonArray>();
    
    // If no streams loaded, create default ones (same as menu system)
    if (webStreamCount == 0) {
        // Default stream data
        const char* defaultStreams[][2] = {
            {"Jacaranda FM", "https://edge.iono.fm/xice/jacarandafm_live_medium.aac"},
            {"Pretoria FM", "https://edge.iono.fm/xice/362_medium.aac"},
            {"Lekker FM", "https://zas3.ndx.co.za:8002/stream"},
            {"Groot FM", "https://edge.iono.fm/xice/330_medium.aac"},
            {"RSG", "https://28553.live.streamtheworld.com/RSGAAC.aac"}
        };
        
        for (int i = 0; i < 5; i++) {
            JsonObject stream = array.createNestedObject();
            stream["name"] = defaultStreams[i][0];
            stream["url"] = defaultStreams[i][1];
        }
    } else {
        // Save existing streams
        for (int i = 0; i < webStreamCount; i++) {
            JsonObject stream = array.createNestedObject();
            stream["name"] = webStreams[i].name;
            stream["url"] = webStreams[i].url;
        }
    }
    
    File file = SPIFFS.open("/streams.json", "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.println("Streams saved to JSON file");
    } else {
        Serial.println("Failed to save streams to file");
    }
}
