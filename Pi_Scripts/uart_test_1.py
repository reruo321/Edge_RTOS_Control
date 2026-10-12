import asyncio
import serialx

SERIAL_PORT = '/dev/ttyAMA0'
BAUD_RATE = 115200

async def async_rx_task(serial):
    try:
        while True:
            data = await serial.readline()
            if data:
                message = data.decode('utf-8', errors='ignore').strip()
                print(f"STM32 said: {message}")
    except asyncio.CancelledError:
        pass
    except Exception as e:
        print(f"Error from RX... {e}")

async def tx_task(serial):
    counter = 0
    try:
        while True:
            await asyncio.sleep(0.5)
            data = f"PING --- {counter}\n".encode('utf-8')
            await serial.write(data)
            counter = counter + 1
    except asyncio.CancelledError:
        pass
    except Exception as e:
        print(f"Error from TX... {e}")

async def main():
    async with serialx.async_serial_for_url(SERIAL_PORT, baudrate=BAUD_RATE) as serial:
        rx = asyncio.create_task(async_rx_task(serial))
        tx = asyncio.create_task(tx_task(serial))
        
        try:
            await asyncio.gather(rx, tx)
        except asyncio.CancelledError:
            pass
        finally:
            rx.cancel()
            tx.cancel()
            await asyncio.gather(rx, tx, return_exceptions=True)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Exit the program... Bye!\n")
