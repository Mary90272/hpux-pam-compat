
# HP-UX 8-Character Password Compatibility (PAM)

## This project introduces an HP-UX–compatible 8-character password fallback mechanism on Linux using PAM.
It allows legacy HP-UX migrated users to authenticate with the first 8 characters of their long password, while all other users continue using full modern password hashing.

## Overview

Some legacy systems (e.g., HP-UX) only stored and validated the first 8 characters of user passwords.
After migration to Linux, these users may not be able to authenticate because Linux validates the full-length password.

This project adds an optional fallback for such users by introducing:

A custom PAM module: pam_hpux_compat.so

A dedicated PAM stack: hpux-test

A conditional include in common-auth

A Linux group (legacyhpux) to control which users receive compatibility behavior

Only users added to the legacyhpux group get the 8-character fallback.

File Changes & Paths
1. New PAM Module
/lib64/security/pam_hpux_compat.so

2. New Include Stack
/etc/pam.d/hpux-test


This stack runs:

pam_unix (full password check)

pam_hpux_compat (8-char fallback)

3. Modified Global Entry Point

File edited:

/etc/pam.d/common-auth


Two lines added at the very top:

auth    [success=1 default=ignore]   pam_succeed_if.so user notingroup legacyhpux
auth    include                      hpux-test


Backups stored as:

/etc/pam.d/common-auth.bak.hpux.<timestamp>

How It Works

Only users inside the legacyhpux group will receive HP-UX compatibility:

Linux first tries normal full-length password via pam_unix

If it fails and user is in the group, fallback checks first 8 characters

If first 8 chars match the stored base hash, authentication succeeds

This allows seamless migration from HP-UX to Linux.

Managing Users
Enable HP-UX fallback for a user
sudo usermod -aG legacyhpux <user>

Set stored password to HP-UX format (first 8 chars only)
echo "<user>:$(openssl passwd -6 <first8>)" | sudo chpasswd -e

Disable fallback
sudo gpasswd -d <user> legacyhpux

Verification
SSH test (password only)
ssh -o PreferredAuthentications=password -o PubkeyAuthentication=no <user>@localhost

su test
su - <user>


Enter long password. If the first 8 characters match the stored base, login succeeds.

Rollback

To restore the latest original common-auth:

sudo mv /etc/pam.d/common-auth.bak.hpux.<timestamp> /etc/pam.d/common-auth


This removes the compatibility layer entirely.

Included Files in Archive

Inside hpux_pam_review_<timestamp>.tar.gz:

common-auth.current

common-auth.bak.hpux.*

hpux-test.current

pam_hpux_compat.so

(optional) pam_hpux_compat.c, pam_check.c

INFO.txt

README.txt

These represent all active and original files for auditing and rollback.

Security Notes

Fallback behavior is not enabled globally—only for users in legacyhpux.

Full-length password validation always runs first.

Fallback only checks first 8 characters when necessary.

Backups ensure easy and safe rollback.
