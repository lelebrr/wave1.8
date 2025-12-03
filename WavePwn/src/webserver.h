#pragma once

// Inicializa o webserver HTTP + WebSocket + OTA seguro.
void webserver_start();

// Envia estatísticas em tempo real via WebSocket para o dashboard.
void webserver_send_stats();