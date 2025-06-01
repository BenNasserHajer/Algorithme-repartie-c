####  ⭐ ***Instructions pour l’exécution :***

1. ***Compilation des fichiers***

```js

   gcc -o coordinator coordinator.c -lpthread
   gcc -o pro1 pro1.c -lpthread
   gcc -o pro2 pro2.c -lpthread
   gcc -o pro3 pro3.c -lpthread
   gcc -o pro4 pro4.c -lpthread

```
   
2. ***Lancement du coordinateur***

```js

./coordinator

```

3. ***Lancement des processus distribués***  

```js

./pro1
./pro2
./pro3
./pro4

```

<strong> ***Resultat*** :</strong>

Horloge Scalaire : 


![versionc-scalaire](https://github.com/user-attachments/assets/daabe633-ae11-4f22-8d59-fb702d1493da)



Horloge Vectoriel:


![versionc-vctor](https://github.com/user-attachments/assets/7a5e4573-527c-4d47-8442-8cc5e8d9bc67)


Horloge Matricielle :


![versionc-matrice](https://github.com/user-attachments/assets/3bf43b09-4ac4-408f-8fdc-508503bf7aa7)
![versionc-matrice2](https://github.com/user-attachments/assets/d95fd1ff-0d70-4deb-a319-f6fd354b16a8)




