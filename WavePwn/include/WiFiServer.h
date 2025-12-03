#pragma once

// Compatibilidade para projetos que incluem "WiFiServer.h" diretamente.
// Em alguns ambientes/versões do core ESP32, a classe WiFiServer é
// disponibilizada por WiFi.h. Este wrapper garante que o include
// antigo continue compilando.
#include <WiFi.h>