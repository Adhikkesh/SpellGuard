# SymSpellChecker

SymSpellChecker is a lightweight spell-checking application powered by the SymSpell algorithm and Trie data structure. The project features a GUI built with Qt and is fully containerized using Docker.

---

## Requirements

### Ubuntu:
- Docker & Docker Compose:
  ```bash
  sudo apt install -y docker.io docker-compose
  sudo usermod -aG docker $USER
  ```

- X11 Display (for GUI support):
  ```bash
  sudo apt install -y xorg mesa-utils
  ```

### Windows (WSL2 + Docker Desktop + X Server):
- **WSL2** (Ubuntu 20.04/22.04 recommended):  
  [Install instructions](https://learn.microsoft.com/en-us/windows/wsl/install)

- **Docker Desktop** with WSL2 integration enabled:  
  [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/)

- **X Server for Windows** (for GUI apps inside WSL2):  
  - Recommended: [VcXsrv](https://sourceforge.net/projects/vcxsrv/) or [Xming](https://sourceforge.net/projects/xming/)

---

## Getting Started

### 1. Clone the repository
```bash
git clone https://github.com/<username>/SymSpellChecker.git
cd SymSpellChecker
```

### 2. Enable Docker GUI Access

#### On Ubuntu:
```bash
xhost +local:docker
```

#### On Windows (WSL2):
- Start **VcXsrv** or **Xming**.
- Inside WSL2 terminal, set the display:
  ```bash
  export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
  export LIBGL_ALWAYS_INDIRECT=1
  ```

> Tip: You can add these export lines to your `~/.bashrc` or `~/.zshrc` to make them permanent.

### 3. Build & Launch the App (Both Ubuntu & Windows)

```bash
docker-compose up --build
```

- The app will load a dictionary (~370k words) and word frequency list (~333k entries) before launching the GUI.

### 4. Shut down
Press `Ctrl+C` to stop, and then:
```bash
docker-compose down
```

---

## Project Structure

- `CASE STUDY.pro`: Qt project config
- `SymSpellChecker.cpp`: C++ core logic (SymSpell + Trie)
- `index.html`: GUI interface via Qt WebEngine
- `words_alpha.txt`: Dictionary dataset
- `english_word_frequency.csv`: Word frequency data
- `Dockerfile`: Container build setup
- `docker-compose.yml`: Docker orchestration
- `.gitignore`: Ignores generated build files

---

## Common Issues

- **No GUI appears**:
  - Ensure you started `xhost +local:docker` (Ubuntu) or your X server (Windows).
  - Confirm `$DISPLAY` is correctly set.

- **OpenGL errors**:
  - Ensure `mesa-utils` (Ubuntu) is installed and GPU drivers are present.
  - On Windows, make sure `VcXsrv/Xming` is running with OpenGL option enabled.

- **Permission denied (Docker)**:
  ```bash
  sudo docker-compose up --build
  ```

---

## Notes
- Tested on Ubuntu 22.04 and Windows 11 (WSL2).
- Uses C++11 and Qt 5.15.
- On macOS, use Docker Desktop + XQuartz with similar X11 setup.
