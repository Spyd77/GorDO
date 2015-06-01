## Synopsis

This project uses an Arduino UNO or Arduino Mega with a SIM900 shield/module and a Relay shield/module to attain the following objectives:
-Be able to call a phone number that automatically switches a relay.
-The received call is never picked up. As soon as the call is received, is dropped so no phone charges are made by firing the relay.
-Only authorized phone numbers will work, other callers will be ignored.
-Phonebook for at least 64 entries.
-Phonebook management done via SMS messages

## Motivation

I wanted to do this project because I hate RF receivers and their poor performance. This is a more useful (and cheaper at the long run) solution.
Plus, it has been a blast making it work.

## Installation

You only need the appropiate hardware (Arduino + SIM900 + Relay) and the Arduino IDE you can download from http://www.arduino.cc/
This project so far only uses libraries that come standard with the IDE installation

## License

License is still undecided, but the TL;DR version is do whatever you want with this, but if you fix things or add features, please contribute, and is you're using it, leave me a note with your impressions.