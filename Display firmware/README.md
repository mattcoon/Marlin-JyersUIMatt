# Updating the Display

To update the graphics and icons on the display:

Please check the type of motherboard of your display (you have 2 pictures of the possible motherboard):

Case A : you have an original DWIN Display :

- Copy one of the `DWIN_SET`s from the Firmware Sets folder to an SD card. Remove the identifier so the folder is just named `DWIN_SET`. Remove the back cover of the display and insert the card into the MicroSD slot on the back of the display unit's PCB.
- Power on the machine and wait for the screen to change from blue to orange (ca. 10 à 20s).
- Power off the machine.
- Remove the SD card from the back of the display.
- Power on to confirm a successful flash.

## Troubleshooting
If flashing is not working (e.g. the blue screen only flashes for a second before the orange screen):
- Reformat the SD card in FAT32 with 4096 bytes allocations

Case B : you have a DACAI Display :

- Copy  the `private` folder from the Display firmware folder to an SD card. Remove the back cover of the display and insert the card into the MicroSD slot on the back of the display unit's PCB.
- Power on the machine and wait. The screen change to blue with a progress bar (picture : Nouvel-ecran-en-cours-de-flash).
- Wait Progress bar to 100% and the end of this process. After Power off the machine.
- Remove the SD card from the back of the display.
- Power on to confirm a successful flash.

## Troubleshooting
If flashing is not working (e.g. the blue screen only wihtout progress bar):
- Reformat the SD card in FAT32 with 4096 bytes allocations


# Mise à jour de l'affichage

Pour mettre à jour les graphiques et les icônes à l'écran :

Veuillez verifier le type de carte mère de votre écran (vous avez 2 images pour les reconnaître):

Cas A : vous disposez d'un DWIN Display :

- Copiez un des `DWIN_SET`s du dossier Firmware Sets ( Custom pour V203j et sup) sur une carte SD. Supprimez l'identifiant pour que le dossier s'appelle simplement "DWIN_SET". Retirez le couvercle arrière de l'écran et insérez la carte dans la fente MicroSD à l'arrière du PCB de l'unité d'affichage.
- Allumez la machine et attendez que l'écran passe du bleu à l'orange (env. 10 à 20s).
- Mettez l'appareil hors tension.
- Retirez la carte SD de l'arrière de l'écran.
- Allumez pour confirmer un flash réussi.

## Dépannage
Si le clignotement ne fonctionne pas (ex : l'écran bleu ne clignote qu'une seconde avant l'écran orange) :
- Reformatez la carte SD en FAT32 avec des allocations de 4096 octets

Cas B : vous disposez d'un DACAI Display :

- Copiez le dossier `private` du dossier du firmware de l'écran sur une carte SD. Retirez le couvercle arrière de l'écran et insérez la carte dans la fente MicroSD à l'arrière du PCB de l'unité d'affichage.
- Allumez la machine et attendez. L'écran passe au bleu avec une barre de progression (photo : Nouvel-ecran-en-cours-de-flash).
- Attendez que la barre de progression soit à 100 % et que le processus soit terminé. Ensuite éteignez votre imprimante.
- Retirez la carte SD de l'arrière de l'écran.
- Allumez pour confirmer un flash réussi.

## Dépannage
Si le clignotement ne fonctionne pas (par exemple, l'écran bleu uniquement sans barre de progression) :
- Reformatez la carte SD en FAT32 avec des allocations de 4096 octets
