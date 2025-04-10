# Panasonic SANE Backend Patch for scanservjs

I managed to get a **Panasonic scanner working on Linux** through **SANE** in combination with **scanservjs**.

---

## 🚨 Known Issues

There are two global issues with the Panasonic driver:

1. The **Panasonic driver and SANE backend return an incorrect list of resolutions**, causing `scanservjs` to fail when parsing them.
2. `saned` has issues handling the **ring buffer** when working with Panasonic devices. It sends incorrect data to `scanimage`.  
   This is **not a problem if you're running scanservjs locally**, outside of Docker, and not using the `net` backend.

---

## ✅ Requirements

You need **SANE version > 1.3.1**. Install it with:

```bash
sudo add-apt-repository ppa:sane-project/sane-release
sudo apt update
sudo apt install libsane libsane-common sane-utils
```

---

## ⚙️ Installation & Setup

### 1. Clone the official SANE backends

```bash
git clone https://gitlab.com/sane-project/backends.git
cd backends
```

Copy the patched `saned.c` from this repository into the `frontend` folder:

```bash
cp PATH_TO_THIS_REPOSITORY/saned.c frontend/saned.c
```

---

### 2. Build the backend

Run the standard SANE build process:

```bash
./autogen.sh
./configure
make
```

---

### 3. Install Panasonic Driver

Download and extract the Panasonic driver:

```bash
cd backend
wget https://www.psn-web.net/cs/support/fax/common/file/Linux_ScanDriver/panamfs-scan-1.3.1-x86_64.tar.gz
tar -xvzf panamfs-scan-1.3.1-x86_64.tar.gz
```

Follow the installation instructions from the official PDF guide:  
[Scanner Driver Installation Guide (PDF)](https://www.psn-web.net/cs/support/fax/common/file/Linux_ScanDriver/ScannerDriver_Ubuntu_ENG_011.pdf)

Then copy the driver's SANE backend library:

```bash
cp mccgdi-2.0.10-x86_64/sane-backend/libsane-panamfs.so.1.3.1 ./libsane-panamfs.real.so
```

---

### 4. Embed the Driver into a Proxy Library

Convert the library to a C header:

```bash
xxd -i libsane-panamfs.real.so > libsane-panamfs.real.h
```

Compile the proxy wrapper:

```bash
gcc -shared -fPIC -o libsane-panamfs.so.1.3.1 panamfs.c -ldl
```

Install it to your system:

```bash
cp -f libsane-panamfs.so.1.3.1 /usr/lib/sane
```

---

## 🧪 Testing

Run the following to verify your setup:

```bash
scanimage -d net:127.0.0.1:panamfs -L       # Lists available devices
scanimage -d net:127.0.0.1:panamfs -A       # Shows device options (check "resolution")
```

---

## 🌐 Enabling saned (net driver)

To enable `saned` for remote scanning:

1. Ensure `saned.c` is in the `frontend` folder.
2. Rebuild:

```bash
make
```

3. Replace the system `saned` binary:

```bash
cd frontend/.libs
chmod +x saned
cp -f saned /usr/bin/
cp -f saned /usr/sbin/
```

---

### 📄 Full Scan Command Example

```bash
scanimage -d net:127.0.0.1:panamfs:libusb:xxx:xxx \
  --mode Color --resolution 100 \
  -l 0.9 -t 0.9 -x 214.9 -y 296 \
  --format tiff --brightness 3 --contrast 3 \
  -o preview.tif
```

---

## ✅ Supported Panasonic Models

- KX-MC6000  
- KX-MC6015  
- KX-MC6020  
- KX-MC6040  
- KX-MC6255  
- KX-MC6260  
- DP-MC210  
- KX-MB3000 / DP-MB300  
- KX-MB2000  
- KX-MB2060  
- KX-MC6280N  
- KX-MB1500  
- KX-MB2200 / 2500, DP-MB250 / 310  
- KX-MB2085 / 2090  
- KX-MB1600  
- KX-MB2100  
- KX-MB2571, DP-MB251 / 311  
- DP-MB500  

---

Let me know if you want to add badges, licensing info, or convert this into a full GitHub project layout!
