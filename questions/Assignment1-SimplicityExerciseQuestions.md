Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer: The current drawn when a single LED is on with GPIO set to StrongAlternateStrong is 5.37 mA.


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer: The current drawn when a single LED is on with GPIO set to WeakAlternateWeak is 5.36 mA.


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate. Extra credit is avilable for this question and depends on your answer.**
   Answer:  drive strength effects slew rate and not the current.
   
			There is not much difference between the current values measured in the cases mentioned above.
		   From the schematics we know that a 3000 ohm resistance is in series with LED. It has a supply voltage
		   of 3.3V and we also know that voltage drop acorss LED is about 1.8V. Thus according to ohms law,
		   
		   V = I*R
		   I = V / R
		     = (3.3 - 1.8) / 3000
			 = 0.5 mA
			 
			From the observations on Energy profiler when LED0 is on and set with StrongAlternateStrong, the current when
			LED is on is 5.40mA and when the LED is off is 4.73mA. Thus the current flowing through LED is 0.67mA.
			
			Thus, both the therotical and practical values authenticate the values of current consumed by LED.


**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: With the WeakAlternateWeak drive strength setting, the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% is 4.88mA.


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: With the WeakAlternateWeak drive strength setting, the average current for 1 complete on-off cycle for 2 LED with an on-off duty cycle of 50% is 4.96mA.


