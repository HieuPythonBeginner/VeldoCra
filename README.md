# VeldoCra Language Support for VSCode

A complete VSCode extension for VeldoCra programming language.

## Features

### 1. Syntax Highlighting
Full syntax highlighting for VeldoCra language including:
- Keywords: `verdict`, `fail`, `scale`, `evolve`, `limit`, `sustain`, `manifest`, `clm`, `ee_inf_layers`, `race`, `ability`, `summon`, `print`, `return`
- Types: `string`, `number`, `bool`, `array`, `dict`, `void`
- Comments (line with `#` and block with `/* */`)
- Strings (single and double quotes)
- Numbers
- Functions

### 2. File Icons
Custom dragon icon for `.vel` files in VSCode file explorer.

### 3. Language Configuration
- Auto-closing brackets and quotes
- Bracket matching
- Indentation rules
- Folding markers

## Installation

### Method 1: Local Development
1. Copy this folder to your VSCode extensions folder:
   - Windows: `%USERPROFILE%\.vscode\extensions\`
   - Linux: `~/.vscode/extensions/`
   - macOS: `~/.vscode/extensions/`

2. Restart VSCode

### Method 2: Package and Install
1. Install vsce (VS Code Extension Manager):
   
```
bash
   npm install -g vsce
   
```

2. Package the extension:
   
```
bash
   vsce package
   
```

3. Install the .vsix file:
   
```
bash
   code --install-extension veldora-language-support-1.0.0.vsix
   
```

## Usage

### Syntax Highlighting
Open any `.vel` file and syntax highlighting will be automatically applied.

### Enable Icon Theme
Add to your VSCode settings (`settings.json`):
```
json
{
  "workbench.iconTheme": "veldora-dragon-icons"
}
```

### Enable Language Features
The extension will automatically:
- Recognize `.vel` files
- Apply syntax highlighting
- Show custom icons

## File Structure

```
veldora-language-support/
├── package.json                 # Extension manifest
├── language-configuration.json  # Language settings
├── syntaxes/
│   └── veldora.tmLanguage.json # Syntax highlighting rules
├── icons/
│   ├── veldora-icons.json      # Icon theme definition
│   ├── veldora.ico             # Windows icon
│   └── VeldAni.png            # Source icon image
└── README.md                   # This file
```

## VeldoCra Language Example

```
vel
# Example VeldoCra code
race DemonLord {
    ability create(name: string) -> DemonLord
        limit power := 9999
        return manifest_demon(name, power)
    
    ability awaken(self)
        verdict power > 5000
            print("The Demon Lord awakens!")
        fail
            print("The Demon Lord stirs...")
}

lord := manifest(DemonLord)
summon(lord.awaken)
```

## License

MIT License - VeldoCra Programming Language
