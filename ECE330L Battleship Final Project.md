Gage Munt, Isaac Hager  
Professor Wilkinson   
ECE330L   
Final Project 

# Battleship STM32

# The Most Difficult Part

In our opinion, the most challenging aspect of this project was implementing adjustable brightness and dimming settings. Initially, our group attempted the suggested byte map based approach, which proved significantly more difficult than anticipated and hindered our progress for nearly two lab sessions. We mainly struggled with the variable states of each segment and how to represent those over just bytes. We successfully were able to display a provided map but only with ON and OFF segments. We eventually opted for the map struct with two arrays instead. From here, we were able to rely on an enum that defined ON, OFF, DIM, and BLINK which made it much easier to handle individual segments. In the end, this was our functional solution and was much easier to understand when debugging. 