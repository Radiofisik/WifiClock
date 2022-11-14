
import React, { FC } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useRouterTab, useLayoutTitle } from '../components';

import DeviceSettingsRestForm from './DeviceSettingsRestForm';
import LightMqttSettingsForm from './LightMqttSettingsForm';
import LightStateWebSocketForm from './LightStateWebSocketForm';
import LogTab from './LogTab';

const Clock: FC = () => {
  useLayoutTitle("Wifi clock");
  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="logs" label="Logs" />
        <Tab value="device-settings" label="Device settings" />
        {/* <Tab value="socket" label="WebSocket Example" /> */}
        {/* <Tab value="mqtt" label="MQTT Settings" /> */}
      </RouterTabs>
      <Routes>
        <Route path="logs" element={<LogTab />} />
        <Route path="device-settings" element={<DeviceSettingsRestForm />} />
        {/* <Route path="mqtt" element={<LightMqttSettingsForm />} /> */}
        {/* <Route path="socket" element={<LightStateWebSocketForm />} /> */}
        <Route path="/*" element={<Navigate replace to="logs" />} />
      </Routes>
    </>
  );
};

export default Clock;
