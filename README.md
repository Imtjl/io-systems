# 🖥️ IO-Systems Course Repository

Welcome to the **IO-Systems** course repository! This repo contains labs and
projects exploring low-level I/O systems, from bare-metal programming to Linux
drivers. 🛠️

---

## 📂 Repository Structure

```bash
.
├── opensbi-console-menu # lab1: bare metal i/o with OpenSBI
└── coming soon...
```

---

## 🚀 Lab 1: OpenSBI Console Menu

In this lab, you'll work at the **loader level**, writing bare-metal C code that
interacts with **OpenSBI** (between firmware and OS levels). You'll implement
basic I/O functions and create a menu-driven interface to interact with OpenSBI.

### 🛠️ How to Run

1. Clone the repository:

```bash
git clone https://github.com/your-username/io-systems.git
cd io-systems
```

2. Navigate to the lab directory:

```bash
cd opensbi-console-menu
```

3. Run the script:

```bash
./script.sh
```

That's it! The script will build the kernel, download OpenSBI (if needed), and
launch QEMU. 🎉
