# Environment File Security Migration

## ⚠️ Security Alert

Your `.env` file previously lived inside the repository, which is **not secure** for a public repo. API keys in `.env` files should never be committed to version control.

## ✅ What Changed

**Old Location (INSECURE):**
```
c:\Adastrea\Plugins\AdastreaDirector\Python\.env
```

**New Location (SECURE):**
```
Windows: %USERPROFILE%\.adastrea\.env
Linux/Mac: ~/.adastrea/.env
```

## 🚀 Migration Steps

### Automatic Migration (Recommended)

Run the migration script:

```powershell
cd "c:\Adastrea\Plugins\AdastreaDirector\Python"
python migrate_env.py
```

This script will:
1. ✅ Copy your existing `.env` file to the secure location
2. ✅ Remove the `.env` file from the repository
3. ✅ Create a backup just in case
4. ✅ Verify everything works

### Manual Migration (Alternative)

If you prefer to do it manually:

1. **Find your API key:**
   - Open: `c:\Adastrea\Plugins\AdastreaDirector\Python\.env`
   - Copy your `GEMINI_KEY` or `OPENAI_API_KEY` value

2. **Create secure location:**
   ```powershell
   mkdir "$env:USERPROFILE\.adastrea"
   ```

3. **Create new .env file:**
   ```powershell
   notepad "$env:USERPROFILE\.adastrea\.env"
   ```

4. **Add your configuration:**
   ```ini
   # Adastrea Director Configuration
   GEMINI_KEY=your_actual_api_key_here
   ```

5. **Delete old file from repo:**
   ```powershell
   cd "c:\Adastrea\Plugins\AdastreaDirector\Python"
   del .env
   ```

## 🔒 Security Improvements

### Before (❌ Insecure)
- `.env` file inside repository
- Could be accidentally committed
- API keys exposed in public repo
- Security risk

### After (✅ Secure)
- `.env` file outside repository in user's home directory
- Cannot be committed to git
- API keys stay private
- Follows security best practices

## 📝 Updated Code

All Python scripts now automatically use the secure location:

- `rag_query.py` - Updated
- `setup_wizard.py` - Updated
- `setup_interactive.py` - Updated
- New utility: `env_config.py` - Handles secure path resolution

## 🛡️ Git Protection

Added to `.gitignore`:
```gitignore
# Environment files with secrets
.env
.env.*
!.env.template
```

This ensures:
- ✅ `.env` files are never committed
- ✅ `.env.template` (without secrets) stays in repo as reference
- ✅ All `.env.*` variants are blocked

## 📍 Finding Your .env File

To find your secure `.env` location:

```powershell
python -c "from env_config import get_env_file_path; print(get_env_file_path())"
```

Or in PowerShell:
```powershell
echo "$env:USERPROFILE\.adastrea\.env"
```

## ✅ Verification

After migration, test that everything works:

```powershell
cd "c:\Adastrea\Plugins\AdastreaDirector\Python"

# Check configuration
python setup_wizard.py

# Or run interactive setup
python setup_interactive.py
```

## 🆘 Troubleshooting

### "API key not found"
- Make sure you ran the migration script
- Check that your API key is in the new location
- Verify the file isn't empty or still using template placeholders

### "Permission denied"
- Make sure you have write access to `%USERPROFILE%\.adastrea\`
- Try running as administrator (not usually needed)

### "Module not found: env_config"
- Make sure you're in the correct directory
- The `env_config.py` file should be in the same folder as other Python scripts

### Want to check the file location?
```powershell
python -c "from env_config import get_env_file_path; print(get_env_file_path())"
```

## 📚 For New Users

If you're setting up for the first time:

1. Run the interactive setup:
   ```powershell
   cd "c:\Adastrea\Plugins\AdastreaDirector\Python"
   python setup_interactive.py
   ```

2. The setup will automatically create the `.env` file in the secure location

3. Follow the prompts to add your API key

## 🔄 Updating API Keys

To change your API key later:

```powershell
notepad "$env:USERPROFILE\.adastrea\.env"
```

## ⚠️ Important Notes

- **Never commit** `.env` files to git
- **Never share** your API keys publicly
- **Backup** your `.env` file separately (not in the repo)
- **Regenerate** API keys if accidentally exposed

## 📞 Support

If you encounter issues:
1. Check the secure file exists: `type "$env:USERPROFILE\.adastrea\.env"`
2. Verify permissions: Make sure you can read/write to the file
3. Re-run migration if needed: `python migrate_env.py`

---

**Migration Date:** 2025-12-13  
**Status:** Security improvement - Required  
**Impact:** All Python scripts updated to use secure location
