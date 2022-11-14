import { FC, useState } from 'react';

import { Button, Checkbox, Slider, Typography } from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';

import { SectionContent, FormLoader, BlockFormControlLabel, ButtonRow, MessageBox, ValidatedTextField } from '../components';
import { updateValue, useRest } from '../utils';

import * as AppApi from './api';
import { DeviceSettings } from './types';
import { ValidateFieldsError } from 'async-validator';

const LightStateRestForm: FC = () => {
  const [fieldErrors, setFieldErrors] = useState<ValidateFieldsError>();
  const {
    loadData, saveData, saving, setData, data, errorMessage
  } = useRest<DeviceSettings>({ read: AppApi.readDeviceSettings, update: AppApi.updateDeviceSettings });

  const updateFormValue = updateValue(setData);

  const handleChange = (event: Event, newValue: number | number[]) => {
    // debugger;
    const defaultData =
    {
      welcomeMessage: "",
      dayBrightness: 0,
      nightBrightness: 0,
      startNightHour: 0,
      endNightHour: 0
    };
    const nonNullData = data ?? defaultData;
    const controllName = (event.target as any).name;

    if (Array.isArray(newValue)) {
    let newNumberValueArr = newValue as number[];
      setData({
        ...nonNullData,
        startNightHour: newNumberValueArr[1],
        endNightHour: newNumberValueArr[0]
      });
    }
    else {
      let newNumberValue = newValue as number;
      setData({
        ...nonNullData,
        [controllName]: newNumberValue
      });
    }
  };

  const content = () => {
    if (!data) {
      return (<FormLoader onRetry={loadData} errorMessage={errorMessage} />);
    }

    return (
      <>

        <ValidatedTextField
          fieldErrors={fieldErrors}
          name="welcomeMessage"
          label="Welcome message"
          fullWidth
          variant="outlined"
          value={data.welcomeMessage}
          onChange={updateFormValue}
          margin="normal"
        />

        <Typography id="track-false-slider" gutterBottom>
          Brightness
        </Typography>

        <Slider
          aria-label="Brightness"
          defaultValue={4}
          value={data.dayBrightness}
          name="dayBrightness"
          onChange={handleChange}
          valueLabelDisplay="auto"
          step={1}
          marks
          min={0}
          max={10}
        />

        <Typography id="track-false-slider" gutterBottom>
          Night Brightness
        </Typography>

        <Slider
          aria-label="Nitht Brightness"
          defaultValue={0}
          value={data.nightBrightness}
          name="nightBrightness"
          onChange={handleChange}
          valueLabelDisplay="auto"
          step={1}
          marks
          min={0}
          max={10}
        />

        <Typography id="track-false-slider" gutterBottom>
          Night Time
        </Typography>
        <Slider
          getAriaLabel={() => 'Day hours'}
          value={[data.startNightHour, data.endNightHour]}
          onChange={handleChange}
          valueLabelDisplay="auto"
          max={23}
        />

        <ButtonRow mt={1}>
          <Button startIcon={<SaveIcon />} disabled={saving} variant="contained" color="primary" type="submit" onClick={saveData}>
            Save
          </Button>
        </ButtonRow>
      </>
    );
  };

  return (
    <SectionContent title='Device settings' titleGutter>
      {content()}
    </SectionContent>
  );
};

export default LightStateRestForm;
