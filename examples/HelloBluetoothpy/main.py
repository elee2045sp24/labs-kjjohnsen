from bleak import BleakClient, BleakScanner # get button state from the m5stickc
import threading
import asyncio
import struct

def run_controller():
    # attempt a connection to the bluetooth device
    def callback(sender,data:bytearray):
        number=struct.unpack("<fffh",data)
        print(number)

    async def run():
        while True: # we'll keep doing this until the program ends
            devices = await BleakScanner.discover(1) # short discovery time so it starts quickly
            print("Devices: ",devices)
            for d in devices:
                if d.name == "M5StickCPlus-Kyle": # this approach should work on windows or mac
                    async with BleakClient(d) as client:
                        await client.start_notify("82a7e967-7504-4f75-a68e-57c2803d8f41",callback)
                        while True:
                            await asyncio.sleep(1)
                        break #we are done
    asyncio.run(run())
t = threading.Thread(target=run_controller) 
t.start() 
print("started")
t.join()
print("end")