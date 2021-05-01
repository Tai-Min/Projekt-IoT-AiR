#pragma once

/**
 * @brief Init WiFi.
 * 
 * This function will initialize WiFi stack and use network's credentials from flash to secure connection.
 * Optionally, this function can ignore flash data and run ESP's smart config to sniff new WiFi credentials.
 * 
 * @param useSmartConfig Whether to use smart config sniffer instead of flash data.
 */
void WiFi_init(bool useSmartConfig);

/**
 * @brief Check whether device is connected to WiFi.
 * @return True if connected.
 */
bool WiFi_isConnected();