class Waveforms {

  LinearInterpolator linearInterpCalc;
  SplineInterpolator splineInterpCalc;

  PolynomialSplineFunction linearInterp;
  PolynomialSplineFunction splineInterp;

  Waveforms() {
    linearInterpCalc = new LinearInterpolator();
    splineInterpCalc = new SplineInterpolator();
    table = WavetableGenerator.gen7(sampleSize, interpWaveformY, interpWaveformX);
  }
  void linearWave() {
    linearInterp = linearInterpCalc.interpolate(dWaveformX, dWaveformY);
  }

  void splineWave() {
    splineInterp = splineInterpCalc.interpolate(dWaveformX, dWaveformY);
  }

  void squareWave() {
    //setting the square wave values
    squareWave = new float[sampleSize+1];
    for (int i =0; i <= sampleSize; i++) {
      int iMap = int(map(i, 0, sampleSize, 0, 9));
      squareWave[i] = (float)dWaveformY[iMap];
      //println(squareWave[i]);
    }
  }

  void updateArrays() {
    interpWaveformX = new int[sampleSize]; //(dis) spacing between samples 
    interpWaveformY = new float[sampleSize+1]; //(val) value of the sample (+2 for start and finish sample)
    for (int i = 0; i<=9; i++) { //updating the array which provides waveform x position values
      dWaveformX[i] = (sampleSize+1)*i/9;
    }
  }

  void updateWaveform() {
    for (int i = 0; i <=sampleSize; i++) {
      float normalizer = 1/(splinePercent + linearPercent + squarePercent);
      float splineAdjusted = (splinePercent*normalizer)*((float)splineInterp.value(i));
      float linearAdjusted = (linearPercent*normalizer)*((float)linearInterp.value(i));
      float squareAdjusted = (squarePercent*normalizer)*squareWave[i];
      interpWaveformY[i] = squareAdjusted + splineAdjusted+linearAdjusted;
    }

    for (int i = 0; i <=sampleSize-1; i++) {
      interpWaveformX[i] = 1;
    }
    ///////////////////////////////////////////////////////////////////////////
    //update wavetable x
    table = WavetableGenerator.gen7(sampleSize, interpWaveformY, interpWaveformX);
    
    for (int i =0; i < voices.size (); i++) {
      voices.get(i).voiceUpdate(table);
    }
  }
}
