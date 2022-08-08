# Telemetry-System developed by UHEV

This system allows to measure the performance of an electric prototype vehicle. It was specially designed to test and tune the performance of a battery-electric supermilleage vehicle. 

The system allows to measure four variables: Current, Voltage, Wheel-RPM and Motor-RPM.

## Basic Structure

The uC code is uploaded to the uC of preference. It was developed in an Arduino UNO. However it is highly recommended to use a uC with capability for 3 serial ports as they will be needed to communicate with: GPS module, SD module and PC.

The receiver code is a simple .py algorithm that reads the sended data trough the serial port and writes it in a .csv file.

## Final Remarks
It is currently under development and the software is not fully finished, but it should do his work.

Take into account the uC code uses software to measure the period of both the wheel and the motor. In the uploaded schematic the IC MC33039 is used to measure the motor speed. This IC gives a voltage output equivalent to the angular speed. Thus, this is a fix we are working on implementing.

If you have any suggestion or question please contact me: felipearenasuribe@gmail.com
