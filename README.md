
## HP-UX 8-Character Password Compatibility (PAM)

This project adds optional HP-UX–style 8-character password fallback to Linux authentication.
Only users in the legacyhpux group receive fallback behavior; all other users authenticate normally.

# Quick Start (No Source Code Needed)

If you only want to use the HP-UX compatibility — you only need 3 files:

1. pam_hpux_compat.so

The PAM module. Required.
Install it to:

sudo cp pam_hpux_compat.so /lib64/security/
sudo chmod 644 /lib64/security/pam_hpux_compat.so

2. hpux-test.current

The PAM include stack /etc/pam.d/hpux-test
It runs:

pam_unix → pam_hpux_compat


Place it at:

/etc/pam.d/hpux-test

3. common-auth.current

Example of the modified /etc/pam.d/common-auth.

Add the following two lines at the top of your system’s common-auth:

auth    [success=1 default=ignore]   pam_succeed_if.so user notingroup legacyhpux
auth    include                      hpux-test


These lines activate the fallback module only for users in the legacyhpux group.

Operational Model
Enable fallback for a user
sudo usermod -aG legacyhpux <user>

Set password hash from first 8 characters
echo "<user>:$(openssl passwd -6 <first8>)" | sudo chpasswd -e

Disable fallback
sudo gpasswd -d <user> legacyhpux

Verification
SSH (password only)
ssh -o PreferredAuthentications=password -o PubkeyAuthentication=no <user>@localhost

su
su - <user>


If the user’s long password’s first 8 characters match the stored base, login succeeds.

Rollback

Restore your original configuration:

sudo mv /etc/pam.d/common-auth.bak.hpux.<timestamp> /etc/pam.d/common-auth

Security Notes

Fallback applies only to users in legacyhpux.

pam_unix (full password) always runs first.

Fallback is only attempted after normal verification fails.

Backups allow safe and quick rollback.
