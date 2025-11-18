
# HP-UX 8-Character Password Compatibility (PAM)
## Overview
This project provides an HP-UX–compatible 8-character password fallback on Linux using a custom PAM module.
It is designed for migrations where legacy HP-UX user accounts only validated the first 8 characters of their passwords.

Only users placed in the legacyhpux group receive fallback behavior.
All other users authenticate normally with full-length passwords.

📦 Quick Start: Use the Prebuilt PAM Binary

This repository includes a fully working, precompiled PAM module.

You can use it immediately without compiling anything.

Install the ready-to-use module:
sudo cp pam_hpux_compat.so /lib64/security/
sudo chmod 644 /lib64/security/pam_hpux_compat.so


Once copied, the module is ready to be used by PAM.

Minimal setup

If you only want the functionality and do not need to inspect the source:

Install the .so file (above)

Use the provided hpux-test PAM stack

Add the two include lines to common-auth

Add users to legacyhpux as needed

This gives you full HP-UX password compatibility without downloading the entire project.

📁 Full Project / Source Code

If you want auditing, transparency, or the ability to modify the module:

Download or clone the repository to access:

pam_hpux_compat.c (source)

pam_check.c (optional helper source)

pam_hpux_compat.so (ready binary)

hpux-test (PAM stack)

Modified common-auth.current

All original backups

INFO.txt

README.txt

Full tarball of files used during implementation

This allows full code review and reproducibility of the environment.

File Changes & Paths
1. New PAM Module
/lib64/security/pam_hpux_compat.so

2. New PAM Include Stack
/etc/pam.d/hpux-test


Runs:

pam_unix (normal full password)

pam_hpux_compat (8-char fallback)

3. Modified Global Entry Point

/etc/pam.d/common-auth — two lines added at top:

auth    [success=1 default=ignore]   pam_succeed_if.so user notingroup legacyhpux
auth    include                      hpux-test


Backups stored as:

/etc/pam.d/common-auth.bak.hpux.<timestamp>

Operational Model
Enable fallback for specific user
sudo usermod -aG legacyhpux <user>

Set password to match HP-UX's 8-char base
echo "<user>:$(openssl passwd -6 <first8>)" | sudo chpasswd -e

Disable fallback
sudo gpasswd -d <user> legacyhpux

Verification
SSH test:
ssh -o PreferredAuthentications=password -o PubkeyAuthentication=no <user>@localhost

su test:
su - <user>


Enter the long password; if its first 8 chars match the stored base, login succeeds.

Rollback

Restore original common-auth:

sudo mv /etc/pam.d/common-auth.bak.hpux.<timestamp> /etc/pam.d/common-auth

Security Notes

Fallback is not global — only users in legacyhpux receive it.

Modern full-length authentication always runs first.

Fallback is only used after a normal failure.

Backups ensure safe rollback.
