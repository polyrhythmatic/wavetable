class Voice extends Thread implements Instrument {

  Oscil wave;
  ADSR adsr;

  float velocity;
  int pitch;
  boolean keyReleased = false;

  float ampAdjust = 0.8;

  double timeOff;

  Wavetable table; //imported wavetable

    Voice(int tempPitch, int tempVelocity, Wavetable tempTable) {

    velocity = map(tempVelocity, 0, 127, 0, 1); 
    table = tempTable;
    pitch = tempPitch;


    //ADSR(maxAmp, attTime, decTime, susLvl, relTime, befAmp, aftAmp);
    adsr = new ADSR(0.3, attack, decay, sustain, release);
    wave = new Oscil(frequency, 1, table);

    wave.patch(adsr);    // patch everything together up to the final output
  }

  void noteOn(float duration) {
    adsr.noteOn();
    wave.setAmplitude(velocity*ampAdjust);//velocity*ampAdjust);
    wave.setFrequency(6.875 *(pow(2.0, ((3.0+(pitch))/12.0))));
    //wave.patch(out);
    adsr.patch(out);    
  }

  void voiceUpdate(Wavetable table) {
    wave.setWaveform(table);
  }

  boolean voiceOff(int tempOffPitch) {
    //println("this was successful");
    if (pitch == tempOffPitch) {
      keyReleased = true;
      // wave.setFrequency(0);
      // wave.unpatch(out);
      return(true);
    } else {
      return false;
    }
  }

  void noteOff() {
    adsr.noteOff();
    adsr.unpatchAfterRelease(out);
    timeOff = millis();
    //println("success???");
  }

  boolean releaseOver(double tempTimeOff) {
    if (tempTimeOff - timeOff >= (release*1000) && keyReleased) {
      return(true);
    } else {
      return(false);
    }
  }

  void waveOff() {
   // wave.setFrequency(0);
    wave.unpatch(out);
    adsr.unpatch(out);
  }

  void setAmp(int numVoices) {
    if (numVoices > 1) {
      ampAdjust = 0.8/float(numVoices);
      wave.setAmplitude(ampAdjust);
    } else {
      ampAdjust = 0.8;
      wave.setAmplitude(ampAdjust);
    }
   // println("ampadjust "+ampAdjust);
  }
}
