```markdown
# SymSpellChecker

SymSpellChecker is a lightweight spell-checking application powered by the SymSpell algorithm and Trie data structure. The project features a GUI built with Qt and is fully containerized using Docker.

## Requirements (Ubuntu)

- **Docker & Docker Compose**:
  ```bash
  sudo apt install -y docker.io docker-compose
  sudo usermod -aG docker $USER
  ```
  _(Log out and log back in for permissions to apply)_

- **X11 Display (for GUI support)**:
  ```bash
  sudo apt install -y xorg mesa-utils
  ```

## Getting Started

### 1. Clone the repository
```bash
git clone https://github.com/<username>/SymSpellChecker.git
cd SymSpellChecker
```

### 2. Enable Docker GUI Access
```bash
xhost +local:docker
```

### 3. Build & Launch
```bash
docker-compose up --build
```
- The app will load a dictionary (~370k words) and word frequency list (~333k entries) before launching the GUI.

### 4. Shut down
Press `Ctrl+C` to stop, and:
```bash
docker-compose down
```

## Project Structure

- `CASE STUDY.pro`: Qt project config
- `SymSpellChecker.cpp`: C++ core logic (SymSpell + Trie)
- `index.html`: GUI interface via Qt WebEngine
- `words_alpha.txt`: Dictionary dataset
- `english_word_frequency.csv`: Word frequency data
- `Dockerfile`: Container build setup
- `docker-compose.yml`: Docker orchestration
- `.gitignore`: Ignores generated build files

## Common Issues

- **No GUI appears**:
  - Ensure `xhost +local:docker` was run.
  - Verify `$DISPLAY` is set (`echo $DISPLAY` should output `:0`).

- **OpenGL errors**:
  - Ensure you have `mesa-utils` and GPU drivers installed.
  - Check `/dev/dri/` for `card0`.

- **Permission denied (Docker)**:
  ```bash
  sudo docker-compose up --build
  ```

## Notes
- This setup is tested on Ubuntu 22.04.
- C++11 and Qt 5.15 used for development.
- Works best on native Linux. WSL2/macOS may need additional X server setup (e.g., VcXsrv or XQuartz).
