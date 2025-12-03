#include "pwn_grid.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "pwnagotchi.h"
#include "ai/neura9_inference.h"

PwnGrid pwnGrid;

// Serviço/assinatura simples para identificar amigos WavePwn.
static const char* PWNGRID_DEVICE_NAME = "WavePwn";
static const uint16_t PWNGRID_MANUFACTURER_ID = 0x5750; // 'W''P'

// Callbacks de scan para detectar outros WavePwns anunciando presença.
class PwnGridScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
public:
    explicit PwnGridScanCallbacks(PwnGrid* grid) : grid_(grid) {}

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
        if (!advertisedDevice) return;

        // Checa manufacturer data com o ID do PwnGrid.
        if (!advertisedDevice->haveManufacturerData()) return;

        std::string mfg = advertisedDevice->getManufacturerData();
        if (mfg.size() < 4) return;

        uint16_t id = (uint8_t)mfg[0] | ((uint8_t)mfg[1] << 8);
        if (id != PWNGRID_MANUFACTURER_ID) return;

        uint8_t level = (uint8_t)mfg[2];

        String name = advertisedDevice->getName().c_str();
        if (name.isEmpty()) {
            name = advertisedDevice->getAddress().toString().c_str();
        }

        grid_->on_friend_detected(name.c_str());

        Serial.printf("[PwnGrid] Amigo detectado: %s (level=%u)\n",
                      name.c_str(),
                      (unsigned)level);
    }

private:
    PwnGrid* grid_;
};

static NimBLEAdvertising* g_adv = nullptr;
static PwnGridScanCallbacks* g_scan_cb = nullptr;

void PwnGrid::begin() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    Serial.println("[PwnGrid] Inicializando BLE cooperativo (modo defensivo)");

    NimBLEDevice::init(PWNGRID_DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    g_adv = NimBLEDevice::getAdvertising();
    g_adv->setScanResponse(true);

    // Começa com nível SAFE.
    current_level = 0;
    advertise_presence();

    // Scan contínuo para outros WavePwns.
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(300);
    scan->setWindow(200);

    g_scan_cb = new PwnGridScanCallbacks(this);
    scan->setAdvertisedDeviceCallbacks(g_scan_cb, true);

    // 0 = scan infinito (reiniciado pelo próprio NimBLE).
    scan->start(0, nullptr, false);

    Serial.println("[PwnGrid] BLE PwnGrid ativo (broadcast + scan)");
}

void PwnGrid::advertise_presence() {
    if (!g_adv) return;

    NimBLEAdvertisementData advData;
    advData.setName(PWNGRID_DEVICE_NAME);

    // Manufacturer data: [ID LSB][ID MSB][THREAT_LEVEL][RESERVED]
    std::string mfg;
    mfg.push_back((char)(PWNGRID_MANUFACTURER_ID & 0xFF));
    mfg.push_back((char)((PWNGRID_MANUFACTURER_ID >> 8) & 0xFF));
    mfg.push_back((char)current_level);
    mfg.push_back((char)0x00);

    advData.setManufacturerData(mfg);
    g_adv->setAdvertisementData(advData);
    g_adv->start();
}

void PwnGrid::share_threat_level(uint8_t level) {
    if (level > 9) level = 9;
    if (current_level == level) return;

    current_level = level;
    advertise_presence();

    Serial.printf("[PwnGrid] Broadcast threat level atualizado: %u (%s)\n",
                  (unsigned)current_level,
                  NEURA9_THREAT_LABELS[current_level]);
}

void PwnGrid::on_friend_detected(const char* name) {
    Serial.printf("[PwnGrid] Oi, %s — WavePwn amigo na área.\n", name ? name : "(desconhecido)");
}