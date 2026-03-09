# Icon/Logo Association Plan for VeldoCra (.vel files)

## 1. Information Gathered
- Project: VeldoCra - một ngôn ngữ lập trình hệ thống mới
- File extension: `.vel`
- Logo image: `VeldAni.png` (đã có sẵn trong project)
- VSCode extension đã được định nghĩa trong `package.json` với tên "Veldora Dragon Icon Theme"
- Cần tạo icon association cho: VSCode, Windows Explorer, Linux File Managers

## 2. Plan

### Step 1: VSCode Icon Theme Setup
- Cập nhật `package.json` với đường dẫn icon theme chính xác
- Tạo thư mục `icons/` 
- Tạo file `icons/veldora-icons.json` định nghĩa icon cho .vel files

### Step 2: Create VSCode Icons Folder & JSON
- Tạo thư mục `icons/` trong project root
- Tạo file `icons/veldora-icons.json` với proper icon mappings

### Step 3: Windows File Association
- Tạo Windows .ico file (có thể convert từ VeldAni.png)
- Tạo installer script cho Windows (.bat hoặc PowerShell)
- Registry entries để associate .vel với icon và default app

### Step 4: Linux File Association
- Tạo file `veldora-dragon-icon-theme.desktop` cho Linux
- Tạo MIME type XML cho .vel files
- Tạo installer script cho Linux (shell script)

### Step 5: Cross-Platform Installer
- Tạo một unified installer script có thể detect OS và cài đặt tương ứng

## 3. Dependent Files to Create/Edit
- `package.json` - Cập nhật icon theme path
- Tạo `icons/veldora-icons.json` - VSCode icon theme definition
- Tạo `install-icons/` directory với:
  - `windows-install.bat` - Windows installer
  - `linux-install.sh` - Linux installer  
  - `mime-type.xml` - Linux MIME type definition
  - `veldora.desktop` - Linux desktop entry

## 4. Followup Steps
- Test VSCode icon hiển thị đúng
- Test Windows file association
- Test Linux file association
- Tạo hướng dẫn sử dụng trong README

