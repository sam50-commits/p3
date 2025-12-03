# p3

# ✔️ **README.md (Copy-Paste This Into Your GitHub Repo)**

# **Custom Memory Allocator – xv6-riscv Modification**

This repository contains the files and modifications used to add a **custom memory allocator** and system call to **xv6-riscv**.

Because GitHub does not allow uploading certain files (such as Makefile) and because the code depends on the full xv6 directory, **this repository does NOT run independently**.
Instead, the files here must be **copied into the correct places inside a working xv6-riscv clone**.

---

## 📁 **Project File Organization**

Below is the correct placement of each file within the xv6 project folder.

---

## 🟦 **kernel/** (files inside the kernel space)

| File               | Description                         |
| ------------------ | ----------------------------------- |
| `kernel/kalloc.c`  | Add your custom allocator functions |
| `kernel/defs.h`    | Add function declarations           |
| `kernel/syscall.h` | Add system call number              |
| `kernel/syscall.c` | Add system call handler             |
| `kernel/sysproc.c` | Implement system call logic         |

---

## 🟩 **user/** (files used in user space)

| File           | Description                          |
| -------------- | ------------------------------------ |
| `user/user.h`  | Add user-space system call interface |
| `user/usys.pl` | System call stub generator 
|all the test programs I added as well

---

## ⚠️ Important Notes

* After inserting the code into xv6, the project will run in the terminal just like shown in my demo video.

* The `Makefile` in this repository has been renamed to **Makefile.txt** because GitHub’s default `.gitignore` prevented the real Makefile from being uploaded.
  Rename it back to `Makefile` manually when replacing it in xv6.

---

## ▶️ How to Use

1. Clone xv6-riscv:

   ```sh
   git clone https://github.com/mit-pdos/xv6-riscv
   ```
2. Copy each file from this repository into the matching xv6 directory.
3. Rename `Makefile.txt` → `Makefile` (if needed).
4. Build and run:

   ```sh
   make qemu
   ```
