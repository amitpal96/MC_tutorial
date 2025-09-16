#  Neutrino Monte Carlo Simulator with Apptainer

Welcome! 
This repository will help to install **ROOT** and **neutrino Monte Carlo simulation** software inside an independent **Apptainer**:

## Requirements

   - Installed [Apptainer](https://apptainer.org/) in your system
   - Approx 10 GB free space

---
## Apptainer Installation Guide
- I would highly recommend to follow official apptainer installtion instructions at [Installing Apptainer](https://apptainer.org/docs/admin/1.4/installation.html). However some basic commands are given below. If you already have apptainer installed, skip this section.
  - In *Ubuntu*
  ```bash
  sudo apt update
  sudo apt install -y software-properties-common
  ```
  ```bash
  sudo add-apt-repository -y ppa:apptainer/ppa
  sudo apt update
  sudo apt install -y apptainer
  ```
  - In *Fedora*
  ```bash
  sudo dnf install -y apptainer
  ```
  - In *AlmaLinux/CentOS*
  ```bash
  sudo dnf install -y epel-release
  sudo dnf install -y apptainer
  ```
  - In *Mac/Windows*, install apptainer via virtual machine (VM). For Mac follow: [Installation in Mac](https://apptainer.org/docs/admin/1.4/installation.html#mac), for Windows follow: [Installtion in Windows](https://apptainer.org/docs/admin/1.4/installation.html#windows)
  - To check installtion, run
  ```bash
  apptainer --version
  ```
  It should show your apptainer version. For me it was `apptainer version 1.4.2`
---

##  Setup Instructions

### **Step 1: Clone git repository**
```bash
git clone https://github.com/amitpal96/MC_tutorial.git
cd MC_tutorial
```

### **Step 2: Building apptainer sandbox**

Apptainer sandbox can be built in two different ways: *Option A* and *Option B*. *Option A* is more simpler and easier way. 

#### *Option A: Using prebuild sif image*

##### *Step I: Download sif image*
Download `genie.sif` from [Google Drive](https://drive.google.com/drive/u/2/folders/1vAZHhfS5ZzjIlKdCbkXQQTngJK8MBuTV) and place it inside `**MC_tutorial**` directory

##### *Step II: Make sandbox using `genie.sif`*
```bash
apptainer build --sandbox genie_sandbox/ genie.sif
```

##### *Step III: Enter the apptainer shell*
```bash
apptainer shell --writable genie_sandbox/
```

##### *Step IV: Set environment for GENIE*
```bash
cd /opt/mywork/
source do_end_genie.sh
```

##### *Step V: Check genie*

```bash
genie
```
You should see `Version 3.04.00` on your screen

##### *Step VI: For further use of GENIE*
Enter the apptainer shell with
```bash
apptainer shell --writable genie_sandbox/
```
Go to correct path and setup environment
```bash
cd /opt/mywork/
source do_end_genie.sh
```


<details>
<summary>Click here to view Option B steps</summary>

#### *Option B: Build apptainer sandbox from scratch*
This is more complicated and time taking process. I would recommend to go with *Option A*

##### **Step I: Download tar files**
 Download tarballs of all required package from:
 [Google Drive](https://drive.google.com/drive/u/2/folders/1n6KGQXpvhwNZMwsl38GaIteWHjbj1sKn) and place all of them inside `**MC_tutorial**` directory

##### **Step II: Build the first apptainer**
Use the provided **`setup_container1.def`** file to build a sandbox environment called **`sandbox_container1`**:

```bash
apptainer build --sandbox sandbox_container1/ setup_container1.def
```

##### **Step III: Build the second apptainer using **`setup_container2.def`**:**

```bash
apptainer build --sandbox sandbox_container2/ setup_container2.def
```
This step will take **~2 hours** as it builds ROOT from source file.


##### **Step IV: Enter apptainer and check root version**

```bash
apptainer shell --writable sandbox_container2
```
```bash
root
```
This should show `Welcome to ROOT 6.30/02`

##### **Step V: GENIE installtion inside the apptainer**

```bash
cd /opt/GENIE
chmod +x do_end_genie.sh
./do_end_genie.sh
```
This will setup environment for GENIE.

```bash
./configure --prefix=/opt/GENIE_build --enable-atmo --enable-lhapdf6 --with-lhapdf6-lib=/opt/lhapdf_install/lib --with-lhapdf6-inc=/opt/lhapdf_install/include --with-log4cpp-inc=/opt/log4cpp_install/include --with-log4cpp-lib=/opt/log4cpp_install/lib --with-pythia6-lib=/opt/pythia/v6_428/lib --with-pythia6-inc=/opt/pythia/v6_428/inc --with-libxml2-lib=/opt/libxml2_install/lib with-libxml2-inc=/opt/libxml2_install/include/libxml2
```
```bash
make -j4
```

Congratulations! You have finally installed GENIE. If you encounter any **error**, do once
```bash
make clean
make -j4
```
If it does not resolve, contact amit.pal@niser.ac.in.


##### **Step VI: For further use of GENIE**

Once GENIE is installed properly, you just need to do,
```bash
apptainer shell --writable sandbox_container2
```
```bash
cd /opt/GENIE
./do_end_genie.sh
```
This should set up your GENIE. You are good to go.
</details>


### **Step 3: Generate first event using GENIE**
```bash
gevgen -r 3 -n 100 -p 14 -t 1000010020 -e 1.0 --cross-sections gxspl-NUsmall.xml
```
This will create 100 neutrino events. If you do `ls`, you can see two new files have been created: `genie-mcjob-3.status` and `gntp.3.ghep.root`. 