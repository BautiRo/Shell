# Lab: shell

### Búsqueda en $PATH
La familia de exec(3) es una capa "arriba" de execve(2). Es decir que recien los parametros correspondientes y los transforman para terminar usando execve(2).
---
Si puede fallar. En el caso de la shell implementada por mi, lanza un mensaje "exec failed" y luego muestra que no se encontro el file o directory, haciendo mencion al ejecutable que se intento buscar.
### Comandos built-in
cd no podria ser implementado sin ser built-in porque necesitamos cambiar el cwd de la shell, es decir del proceso padre, no de un hijo.
Y en el caso de pwd es parecido, ya que lo que queremos es el cwd del proceso padre.
---

### Variables de entorno adicionales
$ /usr/bin/env
USER=nadie
ENTORNO=nada
	Program: [/usr/bin/env] exited, status: 0 
 (/home/bauti) 
$ /usr/bin/env | grep =nad
USER=nadie
ENTORNO=nada

El resultado es el mismo, pero como se ve en el primer comando, me cambia todas las variables de ambiente por las que les paso, no las agrega.
Una posible implementacion seria conseguir un array con todas las variables de ambiente actuales, agregar las nuevas o modificar las correspondientes, y pasarlos al array envp[]. Ahi si se podria obtener un comportamiento similar al esperado.
---

### Procesos en segundo plano
Cuando el programa recibe un comando que corresponde a un proceso en segundo plano, creo un proceso hijo que va a ejecutar este proceso. Entonces una vez hecho el fork, el hijo va a ejecutar el comando correspondiente, y el padre va a esperar al hijo, pero si no cambia el estado no va a bloquear la ejecucion para que pueda seguir siendo utilizada la shell.
---

### Flujo estándar
Con el simbolo > se redirige la salida. El numero 2 delante es la salida de errores ya que sin numero, si estuviese adelante es la salida estandar. Y al final, el simbolo & es para distinguir a los archivos de los descriptores, ya que sin ese simbolo, buscaria redigirgirlo al archivo "1".

$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
bauti

Invirtiendo el orden de las redirecciones obtengo lo mismo en el archivo de texto.
---

### Tuberías simples (pipes)
El exit code es 0.
 
 Usando bash me devuelve el error correspondiente.
 ➜  shell git:(entrega_shell) ✗ ls /noexiste | grep wfrwfasd
ls: cannot access '/noexiste': No such file or directory
➜  shell git:(entrega_shell) ✗ ls /home | noexiste
zsh: command not found: noexiste

 En caso de que el que falle sea el de la izquierda, el de la derecha tomara como input el output del anterior, por mas que sea un error

 $ asdasd | grep dir
: No such file or directory
 (/home/bauti) 
$ aasdasd | grep fai
exec failed
 (/home/bauti) 
$ fwewasd | grep f
exec failed
: No such file or directory
 (/home/bauti) 


---

### Pseudo-variables
0: 
➜  ~ echo $0
zsh
Muestra el nombre de la shell.

$:
➜  ~ echo $$
21639
Muestra el PID de la shell.

!:
➜  ~ echo $!
0
Muestra el PID del ultimo proceso en background.
---


