# SoundSensor_SPS_DA
project file in \projects\target_apps\dsps\dsps_device\Keil_5


��� �������� ������:
Dsps_585 � ���� ������� �� �������, ��� ���������
Dsps_531 � ���� ������� �� ������� ����, �� ������������� ��� USB KIT
SoundSensor_Fix � ���� ������� �������� ��� ��������������� ����������.

�������� ������ ������� ������ �������, ���������� define � ���������� �������:

1. Dsps_531: ����� ����� ������ define __DA14531__
   2. SoundSensor_Fix: ����� ����� ���������������� 
       �)__DA14531__ ����� ���������� ������������� �� da14531
�) __SoundSensor__ , ��� ����������� ������� � ���, ��� ��� �� USB KIT, �� ���� ��������� ��� ������ ��� usb kit �����������, ��� ������ ��� DUD ����������.
�) __SSFix__ ��������� ��� ��������������� ��� ����������� ����. ������ ��� ����� ���������. ���� ��� ��������� � ������������� ��� SPI
�) __ADCTEST__ �������� ��� ��� ��������� ����� ������ ADC. ��� ��������� ������� ��� ���� �� �������� �� ��������
       �) __BOARD2__ : �� ��������� LDO ����������. ��� ���������� ��� ����� 2, ��� ��� �� ���� ����� �� �������� DCDC. 
