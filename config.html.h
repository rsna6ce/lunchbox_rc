#pragma once
String config_html = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"<meta charset=\"UTF-8\">\n"
"<title>Microcontroller Parameter Setting</title>\n"
"</head>\n"
"<body>\n"
"<div>\n"
"<label>pwm_percent_L1:</label>\n"
"<input type=\"number\" id=\"pwm_percent_L1\" min=\"0\" max=\"100\" value=\"{{pwm_percent_L1}}\">\n"
"<button onclick=\"sendParam('pwm_percent_L1')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>pwm_percent_R1:</label>\n"
"<input type=\"number\" id=\"pwm_percent_R1\" min=\"0\" max=\"100\" value=\"{{pwm_percent_R1}}\">\n"
"<button onclick=\"sendParam('pwm_percent_R1')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>pwm_percent_L2:</label>\n"
"<input type=\"number\" id=\"pwm_percent_L2\" min=\"0\" max=\"100\" value=\"{{pwm_percent_L2}}\">\n"
"<button onclick=\"sendParam('pwm_percent_L2')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>pwm_percent_R2:</label>\n"
"<input type=\"number\" id=\"pwm_percent_R2\" min=\"0\" max=\"100\" value=\"{{pwm_percent_R2}}\">\n"
"<button onclick=\"sendParam('pwm_percent_R2')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>pwm_percent_L3:</label>\n"
"<input type=\"number\" id=\"pwm_percent_L3\" min=\"0\" max=\"100\" value=\"{{pwm_percent_L3}}\">\n"
"<button onclick=\"sendParam('pwm_percent_L3')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>pwm_percent_R3:</label>\n"
"<input type=\"number\" id=\"pwm_percent_R3\" min=\"0\" max=\"100\" value=\"{{pwm_percent_R3}}\">\n"
"<button onclick=\"sendParam('pwm_percent_R3')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_I1:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_I1\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_I1}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_I1')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_O1:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_O1\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_O1}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_O1')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_I2:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_I2\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_I2}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_I2')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_O2:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_O2\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_O2}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_O2')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_I3:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_I3\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_I3}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_I3')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>turn_percent_rate_O3:</label>\n"
"<input type=\"number\" id=\"turn_percent_rate_O3\" min=\"0\" max=\"100\" value=\"{{turn_percent_rate_O3}}\">\n"
"<button onclick=\"sendParam('turn_percent_rate_O3')\">Set</button>\n"
"</div>\n"
"\n"
"<hr>\n"
"\n"
"<details>\n"
"\n"
"<summary> (Wifi settings) </summary>\n"
"\n"
"<div>\n"
"<label>SSID:</label>\n"
"<input type=\"input\" id=\"SSID\" value=\"{{SSID}}\">\n"
"<button onclick=\"sendParam('SSID')\">Set</button>\n"
"</div>\n"
"<div>\n"
"<label>PASS:</label>\n"
"<input type=\"password\" id=\"PASS\" value=\"{{PASS}}\">\n"
"<button onclick=\"sendParam('PASS')\">Set</button>\n"
"</div>\n"
"\n"
"</details>\n"
"\n"
"<script>\n"
"function sendParam(paramName) {\n"
"const input = document.getElementById(paramName);\n"
"const value = (input.value);\n"
"\n"
"const url = `http://{{CURRENT_IPADDR}}/api?ev=config&name=${paramName}&val=${value}`;\n"
"\n"
"fetch(url, {\n"
"method: 'POST'\n"
"})\n"
".then(response => response.text())\n"
".then(data => {\n"
"alert(data);\n"
"})\n"
".catch(error => {\n"
"alert('Error: ' + error.message);\n"
"});\n"
"}\n"
"</script>\n"
"</body>\n"
"</html>\n";