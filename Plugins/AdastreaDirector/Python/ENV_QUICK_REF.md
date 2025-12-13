# 🔒 .env Security - Quick Reference

## Where is my .env file?

**Secure Location:**
```
C:\Users\akuma\.adastrea\.env
```

## Quick Commands

```powershell
# View your .env file
type "$env:USERPROFILE\.adastrea\.env"

# Edit your .env file
notepad "$env:USERPROFILE\.adastrea\.env"

# Verify location
python -c "from env_config import get_env_file_path; print(get_env_file_path())"

# Test everything works
cd "c:\Adastrea"
python test_ue_advanced.py
```

## Your API Key

Your Gemini API key is safely stored at the secure location above.

**Never commit `.env` files to git!** ✅ Protected by .gitignore

## Need Help?

See [ENV_SECURITY_SUMMARY.md](ENV_SECURITY_SUMMARY.md) for complete details.
