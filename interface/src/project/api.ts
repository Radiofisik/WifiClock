import { AxiosPromise } from "axios";

import { AXIOS } from "../api/endpoints";
import { DeviceSettings, LightMqttSettings, LightState, Logs } from "./types";

export function readLightState(): AxiosPromise<LightState> {
  return AXIOS.get('/lightState');
}

export function updateLightState(lightState: LightState): AxiosPromise<LightState> {
  return AXIOS.post('/lightState', lightState);
}

export function readBrokerSettings(): AxiosPromise<LightMqttSettings> {
  return AXIOS.get('/brokerSettings');
}

export function updateBrokerSettings(lightMqttSettings: LightMqttSettings): AxiosPromise<LightMqttSettings> {
  return AXIOS.post('/brokerSettings', lightMqttSettings);
}

export function readDeviceSettings(): AxiosPromise<DeviceSettings> {
  return AXIOS.get('/settings');
}

export function updateDeviceSettings(lightMqttSettings: DeviceSettings): AxiosPromise<DeviceSettings> {
  return AXIOS.post('/settings', lightMqttSettings);
}

export function readLogs(): AxiosPromise<Logs> {
  return AXIOS.get('/logger');
}

export function clearLogs(): AxiosPromise<Logs> {
  return AXIOS.post('/logger', {});
}