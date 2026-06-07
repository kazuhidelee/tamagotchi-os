savedcmd_pet.mod := printf '%s\n'   pet.o | awk '!x[$$0]++ { print("./"$$0) }' > pet.mod
