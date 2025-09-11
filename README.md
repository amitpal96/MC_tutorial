#  Neutrino Monte Carlo Simulator with Apptainer

Welcome! 
This repository provides **container definition files** to build a complete environment for **neutrino Monte Carlo simulations** using:

-  **ROOT**  
-  **GENIE**  
-  **NuWro**

All of these tools are wrapped inside **Apptainer containers** , making the setup portable and reproducible.

---

##  Requirements

Before starting, ensure you have:

-  A **Linux system** with [Apptainer](https://apptainer.org/) installed  
-  At least **X GB free disk space**  
-  Tarballs of required packages:  
  - ROOT  
  - Pythia6  
  - LHAPDF  
  - log4cpp  
  - libxml2  
  - GENIE  
  - NuWro  
  - (and others, as specified in the `.def` file)  

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