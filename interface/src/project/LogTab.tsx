import { FC } from 'react';

import { Typography, Box, List, ListItem, ListItemText, TextareaAutosize, Button } from '@mui/material';

import { ButtonRow, FormLoader, SectionContent } from '../components';
import { Logs } from './types';
import * as AppApi from './api';
import { useRest } from '../utils';
import SaveIcon from '@mui/icons-material/Save';

const LogTab: FC = () => {
  const {
    loadData, saveData, saving, setData, data, errorMessage
  } = useRest<Logs>({ read: AppApi.readLogs, update: AppApi.clearLogs });

  if (!data) {
    return (<FormLoader onRetry={loadData} errorMessage={errorMessage} />);
  }

  return (
    <SectionContent title='Logs' titleGutter>

      <TextareaAutosize
        maxRows={400}
        aria-label="maximum height"
        placeholder="No logs"
        value={data.message}
        style={{ width: 900 }}
      />

      <ButtonRow mt={1}>
        <Button disabled={saving} variant="contained" color="primary" type="submit" onClick={saveData}>
          Clear
        </Button>
        <Button variant="contained" color="primary" type="submit" onClick={loadData}>
          Update
        </Button>
      </ButtonRow>

    </SectionContent>
  );
}

export default LogTab;
