import asyncio
from bleak import BleakScanner
async def run():
    scanner = BleakScanner()
    devices = await scanner.discover(5,return_adv = True)

    for k,v in devices.items():
        print(k,v[0].name,v[1].rssi)
asyncio.run(run())

