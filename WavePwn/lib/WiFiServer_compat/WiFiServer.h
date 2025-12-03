#pragma once

// Compatibilidade para projetos que incluem &quot;WiFiServer.h&quot; diretamente.
// Em alguns ambientes/versões do core ESP32, o cabeçalho pode ter sido
// movido ou reestruturado, mas a classe WiFiServer continua definida em
// WiFi.h. Este wrapper garante que includes antigos continuem compilando.

#include &lt;WiFi.h&gt;