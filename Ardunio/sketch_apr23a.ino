int sine[255];


void setup() 
{ 
     pinMode(0, OUTPUT); 
     pinMode(1, OUTPUT); 
     pinMode(2, OUTPUT); 
     pinMode(3, OUTPUT); 
     pinMode(4, OUTPUT); 
     pinMode(5, OUTPUT); 
     pinMode(6, OUTPUT);
     pinMode(7, OUTPUT); 
    
     float x; 
     float y; 
     for(int i=0;i<255;i++)
     {
          x=(float)i;
          y=sin((x/255)*2*10*PI);
          sine[i]=int(y*31)+76.6;
     }
     Serial.begin(9600);

} 


void loop() 
{ 
     for (int i=0;i<255;i++) 
     { 
          PORTD=sine[i]; 
          Serial.print("ADC Value : ");
          Serial.println(double(PORTD)/double(255)*5);
          delayMicroseconds(10); 
     }
}
