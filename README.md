# Cellworld VR

## 🧠 Overview

**Cellworld VR** is a multi-agent research environment integrating a dedicated server with Meta Quest 3 clients, built using Unreal Engine 5.3.

---

## 📚 Glossary

- [🖥️ System Compatibility](#️-system-compatibility)
- [🌐 Cellworld Server](#-cellworld-server)
- [⚙️ Setup](#-setup)
  - [📁 Cloning This Repository](#-cloning-this-repository-git-lfs-required)
  - [✅ Install Git LFS](#-step-1-install-git-lfs-windows-only)
  - [✅ Clone the Repository](#-step-2-clone-the-repository)
  - [🧪 Verify LFS Setup](#-verify-lfs-setup)
- [📦 Other (Temporary)](#-other-temporary)

---

## 🖥️ System Compatibility

- **Server OS**: Windows 11 (Dedicated Host)
- **Client Device**: Meta Quest 3
- **Engine**: Unreal Engine 5.3.2 (Oculus-VR branch)  
  [→ Engine Source](https://github.com/felixmaldonadoos/UnrealEngine#)

---

## 🌐 Cellworld Server

Make sure to use the [`CellworldServer-dev` branch](https://github.com/felixmaldonadoos/CellworldServer/tree/dev) when working with the server backend.

---

## ⚙️ Setup

### 📁 Cloning This Repository (Git LFS Required)

This repository uses [Git Large File Storage (LFS)](https://git-lfs.com/) to manage large assets such as 3D models and textures.

#### ✅ Step 1: Install Git LFS (Windows Only)

```powershell
winget install --id Git.Git -e --source winget
git lfs install
```

#### ✅ Step 2: Clone the Repository

Make sure to run the following commands to properly clone and pull large files:

```powershell
git clone https://github.com/cellworld/cellworld_vr.git
cd cellworld_vr
git lfs pull
```

If you've already cloned the repo without Git LFS installed:

```powershell
git lfs install
git lfs pull
```

#### 🧪 Verify LFS Setup

To confirm that Git LFS is tracking files correctly:

```powershell
git lfs ls-files
```

---

## 📦 Other (Temporary)

_This section can be used to include temporary notes, future plans, or development todos._
