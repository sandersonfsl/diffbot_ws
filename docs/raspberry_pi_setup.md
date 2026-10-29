# Raspberry Pi 5 (16GB) hardware setup


## OS

Ubuntu Server 24.04, installed directly (no desktop environment).

## SSH on a non-default port

SSH configured to listen on port `3333` instead of the default `22`.

## Networking — prioritize Wi-Fi over Ethernet

`/etc/netplan/50-cloud-init.yaml`, both interfaces configured with static IPs, Wi-Fi given a
lower (preferred) route metric than Ethernet so Wi-Fi wins whenever both are up:

```yaml
network:
  version: 2
  renderer: networkd
  ethernets:
    eth0:
      dhcp4: no
      addresses:
        - <YOUR_IP>/24
      routes:
        - to: default
          via: <YOUR_GATEWAY>
          metric: 100        # lower priority
      nameservers:
        addresses: [8.8.8.8, 1.1.1.1]
      optional: true
  wifis:
    wlan0:
      optional: true
      dhcp4: no
      addresses:
        - <YOUR_IP>/24
      routes:
        - to: default
          via: <YOUR_GATEWAY>
          metric: 50         # higher priority — Wi-Fi wins
      nameservers:
        addresses: [8.8.8.8, 1.1.1.1]
      access-points:
        "<YOUR_SSID>":
          password: "<YOUR_WIFI_PASSWORD>"
```

Apply with `sudo netplan apply`.

## Disable Wi-Fi power save (systemd service)

Wi-Fi power save causes dropped/laggy connections on a robot that needs a stable link for
teleop and ROS2 traffic. Disabled permanently via a oneshot systemd unit instead of a manual
`iw` call that wouldn't survive a reboot.

```sh
sudo apt update
sudo apt install iw
```

`/etc/systemd/system/wifi-powersave-off.service`:

```ini
[Unit]
Description=Disable WiFi power save
After=network-online.target
Wants=network-online.target

[Service]
Type=oneshot
ExecStart=/usr/sbin/iw dev wlan0 set power_save off
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
```

```sh
sudo systemctl daemon-reload
sudo systemctl enable --now wifi-powersave-off.service
```

## Serial port permissions

Add the user to `dialout` so the Arduino's serial port doesn't need `sudo`:

```sh
sudo usermod -aG dialout $USER
```

Group membership only applies to new login sessions — log out/in (or `su - $USER`) to pick
it up without a full reboot.

## Bluetooth joystick (Logitech F710)

```sh
sudo apt update
sudo apt install bluez bluetooth
sudo systemctl enable --now bluetooth
```

Pair via `bluetoothctl`:

```
bluetoothctl
power on
agent on
default-agent
scan on
pair C6:F2:4D:3D:E1:CB
trust C6:F2:4D:3D:E1:CB
connect C6:F2:4D:3D:E1:CB
```

Verify the device shows up as an input device:

```sh
sudo apt install evtest
sudo evtest
```

If the connection is flaky/slow to reconnect, uncomment `FastConnectable = false` in
`/etc/bluetooth/main.conf`.

## udev rules — generic input devices

`/etc/udev/rules.d/81-event.rules`:

```
SUBSYSTEM=="input", KERNEL=="event*", GROUP="input", MODE="0660"
```

```sh
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -aG input $USER   # new session needed to take effect
```

## udev rule — joystick device node

`/etc/udev/rules.d/99-joystick.rules`:

```
SUBSYSTEM=="input", KERNEL=="js*", GROUP="dialout", MODE="0666"
```

```sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Arduino firmware via PlatformIO (not the Arduino IDE)

Lets the Nano firmware be built/flashed from
the CLI, independent of the ROS 2 container, in its own project folder (kept separate from
the `diffbot_firmware` ROS2 package on purpose: one is the Nano-side sketch, the other is the
ROS2-side serial bridge — different build systems, different targets, no reason to couple them).

```sh
mkdir ~/platformio && cd ~/platformio
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
sudo apt update && sudo apt install python3-venv
python3 get-platformio.py
```

Expose the CLI tools:

```sh
sudo mkdir -p /usr/local/bin
sudo ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio
sudo ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio
sudo ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb
pio --version   # sanity check
```

udev rules for PlatformIO's own upload tooling:

```sh
curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
sudo service udev restart
```

Project setup (Arduino Nano w/ ATmega328P):

```sh
pio boards nano   # confirms the board id: nanoatmega328
mkdir -p ~/diffbot/diffbot_firmware_pio/src
cd ~/diffbot/diffbot_firmware_pio
pio project init --board nanoatmega328
touch src/main.cpp   # paste the .ino sketch content here (robot_control.ino / robot_control_inverted.ino)
```

`platformio.ini` — supports both a Nano and an Uno as build targets, switchable via
`default_envs`:

```ini
[platformio]
default_envs = nanoatmega328 ; change to nanoatmega328 or uno

[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328
framework = arduino
upload_speed = 115200
monitor_speed = 115200
lib_deps =
    br3ttb/PID @ ^1.2.1

[env:uno]
platform = atmelavr
board = uno
framework = arduino
upload_speed = 115200
monitor_speed = 115200
lib_deps =
    br3ttb/PID @ ^1.2.1
```

Usage:

```sh
pio run --target upload        # build + flash (uses default_envs)
pio run -e nanoatmega328 -t upload   # explicit env, useful with multiple [env:...] blocks
pio run --target clean
pio device monitor             # serial monitor, e.g. for the motor test sketch
```

## Docker (ROS 2 Humble container on the Pi)

```sh
docker build -t diffbot_humble .
```

```sh
docker run -dt \
  --name diffbot_humble \
  --restart always \
  --network host \
  --privileged \
  -v /dev:/dev \
  -v ~/development:/development \
  diffbot_humble
```

Day to day:

```sh
docker exec -it diffbot_humble /bin/bash   # re-enter
docker start diffbot_humble               # restart if exited
```

## MPU6050 IMU — I2C setup (inside the container)

Dockerfile additions:

```dockerfile
RUN apt-get update && apt-get install -y \
    i2c-tools \
    libi2c-dev \
    python3-smbus
RUN pip3 install smbus2
```
