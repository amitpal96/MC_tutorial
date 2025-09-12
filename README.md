#  Neutrino Monte Carlo Simulator with Apptainer

Welcome! 
This repository provides **container definition files** to build a complete environment for **neutrino Monte Carlo simulations** using:

-  **ROOT**  
-  **GENIE**  
-  **NuWro**

All of these tools are wrapped inside **Apptainer containers** , making the setup portable and reproducible.

## Container Installation Guide
- Visit [Installing Apptainer](https://apptainer.org/docs/admin/1.4/installation.html)
  - In Ubuntu
  ```bash
  sudo add-apt-repository -y ppa:apptainer/ppa
  sudo apt update
  sudo apt install -y apptainer
  ```
  - In Fedora
  ```bash
  sudo dnf install -y apptainer
  ```
  - In AlmaLinux/CentOS
  ```bash
  sudo dnf install -y epel-release
  sudo dnf install -y apptainer
  ```
---

##  Requirements

Before starting, ensure you have:

-  A **Linux system** with [Apptainer](https://apptainer.org/) installed  
-  **Tarballs of required packages:**  
   - ROOT  
   - Pythia6  
   - LHAPDF 
   - log4cpp  
   - libxml2
   - GENIE
   - NuWro (will be cloned from github)  

 **All package tarballs** are available here:  
 [Download from Google Drive](https://drive.google.com/drive/u/2/folders/1n6KGQXpvhwNZMwsl38GaIteWHjbj1sKn)  

Once downloaded, place everything inside a folder, e.g. **`packages/`**.

---

##  Setup Instructions

### **Step 1  Build the Base Container**

Use the provided **`tutorial.def`** file to build a sandbox environment called **`work_container`**:

```bash
#  Build the sandbox container
apptainer build --sandbox work_container tutorial.def
```

### **Step 2  Verify ROOT Installation**

Once the base container is built, open an interactive shell inside it:

```bash
# Enter the container
apptainer shell --writable work_container
```
Once inside the container, check if ROOT is installed and accessible:
```bash
root --version
```

### **Step 3  Build the Base Container**

Use the provided **`final.def`** file to build a sandbox environment called **`final_container`**:

```bash
#  Build the sandbox container
apptainer build --sandbox final_container final.def
```
##  Step 4: Running GENIE and NuWro

After building the final container, you can run **GENIE** inside the sandbox.

### Open apptainer
```bash
apptainer shell --writable final_sandbox
```

### GENIE check
```bash
gevgen --help
```
### NuWro check
```bash
nuwro --version
```

##  New step 1: Installation of prerequisites
```bash
apptainer build --sandbox sandbox_container1/ setup_container1.def
```

##  New	step 1:	Installation of	root and setting environment for GENIE
```bash
apptainer build --sandbox sandbox_container2/ setup_container2.def
```
### Next step 3: GENIE installation
```bash
apptainer shell --writable sandbox_container2
```
```bash
cd /opt/GENIE
chmod +x do_end_genie.sh
./do_end_genie.sh
```

```bash
./configure --prefix=/opt/GENIE_build --enable-atmo --enable-lhapdf6 --with-lhapdf6-lib=/opt/lhapdf_install/lib --with-lhapdf6-inc=/opt/lhapdf_install/include --with-log4cpp-inc=/opt/log4cpp_install/include --with-log4cpp-lib=/opt/log4cpp_install/lib --with-pythia6-lib=/opt/pythia6/v6_428/lib --with-pythia6-inc=/opt/pythia6/v6_428/inc
```
```bash
make -j4
```