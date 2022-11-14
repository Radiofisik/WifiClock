import { FC } from 'react';
import { Navigate, Routes, Route } from 'react-router-dom';

import Clock from './Clock';

const ProjectRouting: FC = () => {
  return (
    <Routes>
      {
        // Add the default route for your project below
      }
      <Route path="/*" element={<Navigate to="clock/logs" />} />
      {
        // Add your project page routes below.
      }
      <Route path="clock/*" element={<Clock />} />
    </Routes>
  );
};

export default ProjectRouting;
