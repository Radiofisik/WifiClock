export interface LightState {
  led_on: boolean;
}

export interface LightMqttSettings {
  unique_id: string;
  name: string;
  mqtt_path: string;
}


export interface DeviceSettings{
  welcomeMessage: string;
  dayBrightness: number;
  nightBrightness: number;
  startNightHour: number;
  endNightHour: number;
}

export interface Logs{
  message: string;
}