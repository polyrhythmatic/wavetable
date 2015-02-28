import org.apache.commons.math3.analysis.polynomials.PolynomialSplineFunction;
import org.apache.commons.math3.analysis.interpolation.SplineInterpolator;
import org.apache.commons.math3.analysis.interpolation.LinearInterpolator;

import themidibus.*;

import ddf.minim.spi.*;
import ddf.minim.signals.*;
import ddf.minim.*;
import ddf.minim.analysis.*;
import ddf.minim.ugens.*;
import ddf.minim.effects.*;

import processing.serial.*;
Serial myPort;

String controllerStringRaw;
String slidersOut = "";

char[] slidersOutChar = new char[50];

boolean presetCalled = false;
boolean buttonPressed = false;
int buttonPressedVal = 0;
int waveformEncoderVal = 0;
int resolutionEncoderVal = 0;
int transitionSpeedVal = 0;
int setButtonVal = 0;
int[] sliderPosition = new int[12];

String midiKeyboard = "Bus 1";

boolean setButton = false;

float frequency = 0;

int sampleSize = 500; 
int sampleSizeTemp = 0;
float linearPercent = 0;
float splinePercent = 1;
float squarePercent = 0;

float attack = 0;
float decay = 0;
float sustain = 0;
float release = 0;
float[] adsrArray = new float[4];

double[] dWaveformX = new double[10]; //X positions for above values
double[] dWaveformY = new double[10]; //double values for interpolation from sliders ([0] and [9] are ZERO)

//for taking interpolation values and feeding them into minim wavetable
int[] interpWaveformX = new int[sampleSize]; //(dis) spacing between samples 
float[] interpWaveformY = new float[sampleSize+1]; //(val) value of the sample (+2 for start and finish sample)

float[] squareWave;//square wave values 

MidiBus slInterface; // interface midibus
MidiBus keyboard; // keyboard midibus

Minim  minim;
AudioOutput out;
Wavetable  table;

ArrayList<Voice> voices;

Waveforms waveform;
Display display;
Save save;
ArrayList<Slider> waveSliders; //waveform sliders
ArrayList<Slider> adsrSliders; //arraylist sliders

void setup() {
  background(0);
  //size(800, 400, P3D);
  size(1400, 700, P3D);

  minim = new Minim(this);
  waveform = new Waveforms();
  display = new Display();
  save = new Save();

  MidiBus.list();
  keyboard = new MidiBus(this, midiKeyboard, -1);
  String portName = "/dev/tty.usbmodem1411";
  myPort = new Serial(this, portName, 115200); // starting new serial communication

  initializeWaveforms();

  voices = new ArrayList<Voice>(); //oscillator arraylist

  out = minim.getLineOut();

  waveSliders = new ArrayList<Slider>(); 
  adsrSliders = new ArrayList<Slider>();

  for (int i = 0; i <=7; i++) {
    waveSliders.add(new Slider((width/16)+i*(width/8))); //creating the waveform array
  }

  for (int i = 0; i<=3; i++) {
    adsrSliders.add(new Slider((2*width/3)+i*(width/12))); //creating the adsr slider array
  }

  attack = map(adsrSliders.get(0).sliderY, height, 0, 0.01, 4);
  decay = map(adsrSliders.get(1).sliderY, height, 0, 0.0001, 10);
  sustain = map(adsrSliders.get(2).sliderY, height, 0, 0.0001, 1);
  release = map(adsrSliders.get(3).sliderY, height, 0, 0.0001, 2);

  ////////////////////////////////////////////////////////////////////////////////////////
  //interpolation initialization
  dWaveformY[0] = 0;//0 val at start and finish of waveform
  dWaveformY[9] = 0;

  waveform.linearWave();//run once to initialize
  waveform.splineWave();
  waveform.squareWave();
  waveform.updateWaveform();

  myPort.write("1");
}

void draw() {
  background(255);

  for (int i = 0; i <=3; i++) {//checking the 
    int tempSliderY = adsrSliders.get(i).sliderY;
    int tempSliderX = adsrSliders.get(i).sliderX;
    strokeWeight(10);
    stroke(128);
    line(tempSliderX, height, tempSliderX, map(tempSliderY, height, 0, height, 3*height/4));
  }

  //updating arrays
  for (int i = 0; i <=7; i++) {//checking the wave sliders to see if they have moved, displaying their position, getting positions in array
    waveSliders.get(i).display();
    dWaveformY[i+1] = map(waveSliders.get(i).sliderY, height, 0, -1, 1);
  }
  for (int i =0; i <voices.size (); i++) {
    if (voices.get(i).releaseOver(millis())) {
      voices.get(i).waveOff();
      voices.remove(i);
    }
  }  

  for (int i =0; i <voices.size (); i++) {
    voices.get(i).setAmp(voices.size());
  }

  //display.interfaceLines();
  display.waveform();
}

void serialEvent(Serial myPort) { 
  String controlStringRaw = myPort.readStringUntil('\n');
  if (controlStringRaw != null) {
    //println(controlStringRaw);
    //controlChange(controlStringRaw);    
    String controllerStringTrimmed = trim(controlStringRaw);
    String[] list = split(controllerStringTrimmed, ','); 
    setButtonVal = int(list[0].trim());
    buttonPressedVal = int(trim(list[1]));

    for (int i = 0; i < 12; i++) {
      sliderPosition[i] = int(list[i+2].trim());
    }

    waveformEncoderVal = int(list[14].trim());
    resolutionEncoderVal = int(list[15].trim());
    transitionSpeedVal = int(list[16].trim());
  }
  controlChange();
  // byte byteOut = byte(2);
  if (presetCalled == true) {
    //print(slidersOut);
    myPort.write(slidersOut);
    //   //myPort.write('2');
    //   //myPort.write('\n');
    //   // for(int i = 0; i < slidersOut.length(); i++){
    //   // //slidersOutChar[i] = slidersOut.charAt(i);
    //   // myPort.write(slidersOut.charAt(i));
    // }
  } else if (presetCalled != true) {
    myPort.write("1");
  }
  presetCalled = false;
}
void controlChange() {
  for (int i = 0; i < 8; i++) {
    //a different parameter
    int changeY = (int) map(sliderPosition[i], 1023, 0, height, 0);
    waveSliders.get(i).midiSlider(changeY);
  }
  if (waveformEncoderVal <= 32 || waveformEncoderVal >= 64) {//adding linear wave
    // int range;
    // if (waveformEncoderVal >= 64) {
    //   range = waveformEncoderVal - 64;
    // } else {
    //   range = waveformEncoderVal + 32;
    // }
    if (waveformEncoderVal <= 32) {
      splinePercent = map(waveformEncoderVal, 0, 32, 1, 0);
    } else if (waveformEncoderVal >= 64) {
      splinePercent = map(waveformEncoderVal, 64, 96, 0, 1);
    } else {
      splinePercent = 0;
    }
  }

  if (waveformEncoderVal >= 0 && waveformEncoderVal <= 32) {//square wave mixing
    squarePercent = map(waveformEncoderVal, 0, 32, 0, 1);
  } else if (waveformEncoderVal > 32 && waveformEncoderVal <= 64) {
    squarePercent = map(waveformEncoderVal, 32, 64, 1, 0);
  } else {
    squarePercent = 0;
  }

  if (waveformEncoderVal >= 32 && waveformEncoderVal <= 64) {//adding spline wave
    linearPercent = map(waveformEncoderVal, 32, 64, 0, 1);
  } else if (waveformEncoderVal >= 64 && waveformEncoderVal <= 96) {
    linearPercent = map(waveformEncoderVal, 64, 96, 1, 0);
  } else {
    linearPercent = 0;
  }
  // println("sq pc: "+squarePercent);
  // println("lin pc: "+linearPercent);
  // println("spl pc: "+splinePercent);


  sampleSizeTemp = int(map(resolutionEncoderVal, 0, 96, 9, 137));
  //println(resolutionEncoderVal);
  //println(sampleSizeTemp);

  for (int i = 0; i < 4; i++) {//setting ADSR values
    int changeY = (int) map(sliderPosition[i+8], 1023, 0, height, 0);
    adsrSliders.get(i).midiSlider(changeY);

    attack = map(adsrSliders.get(0).sliderY, height, 0, 0.01, 4);
    decay = map(adsrSliders.get(1).sliderY, height, 0, 0.0001, 10);
    sustain = map(adsrSliders.get(2).sliderY, height, 0, 0.0001, 1);
    release = map(adsrSliders.get(3).sliderY, height, 0, 0.0001, 1);
  }



  //setting buttons
  //println(setButtonVal);
  //println("buttonpressedval " + buttonPressedVal);
  if (buttonPressed == false && buttonPressedVal > 0) {
    buttonPressed = true;

    if (setButtonVal == 1) {
      //println("setbutton true");
      save.saveValues(buttonPressedVal);
    }

    if (setButtonVal != 1) {//if set button isn't pressed, load user setting
      save.loadValues(buttonPressedVal);
      presetCalled = true;
      // println("load");
    }
  } else if (buttonPressed == true && buttonPressedVal == 0) {
    buttonPressed = false;
  }


  if (sampleSizeTemp >1) { //updating the sample size to correspond with the knob adjustment
    sampleSize = sampleSizeTemp;
  }

  waveform.updateArrays();

  waveform.linearWave();
  waveform.splineWave();
  waveform.squareWave();
  waveform.updateWaveform();
}

void noteOn(Note note) {
  // Receive a noteOn
  // println();
  // println("Note On:");
  // println("--------");
  // println("Channel:"+note.channel());
  // println("Pitch:"+note.pitch());
  //println("Velocity:"+note.velocity());

  for (int i =0; i < voices.size (); i++) {//checking to see if note is already being played (release still on)
    if (voices.get(i).voiceOff(note.pitch())) {
      voices.get(i).noteOff();
      voices.get(i).waveOff();
      voices.remove(i);
    }
  }

  voices.add(new Voice(note.pitch(), note.velocity(), table));
  //println(voices);
  for (int i =0; i < voices.size (); i++) {
    voices.get(i).setAmp(voices.size());
    voices.get(i).noteOn(1);
  }
}
void noteOff(Note note) {
  // Receive a noteOff
  // println();
  // println("Note Off:");
  // println("--------");
  // println("Channel:"+note.channel());
  // println("Pitch:"+note.pitch());
  // println("Velocity:"+note.velocity());

  for (int i =0; i < voices.size (); i++) { //checks voice objects being played to see if they match the noteoff pitch
    if (voices.get(i).voiceOff(note.pitch())) {
      voices.get(i).noteOff();
      //voices.get(i).setAmp(voices.size());
    }
  }
}

void initializeWaveforms() {
  for (int i = 0; i <=sampleSize; i++) {//intialize the waveform array to avoid null error
    interpWaveformY[i] = 0;
  }
  for (int i = 0; i <=sampleSize-1; i++) {//intialize the waveform array to avoid null error
    interpWaveformX[i] = 1;
  }  
  for (int i = 0; i<=9; i++) { //filling the array which provides waveform x position values
    dWaveformX[i] = (sampleSize+1)*i/9;
  }
}

void stop() {
  // always close Minim audio classes when you are done with them
  out.close();
  // always stop Minim before exiting.
  minim.stop();

  super.stop();
}
