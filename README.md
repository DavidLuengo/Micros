# Micros
Repositorio de bolera


**Recordatorio basico de Git:** Git es un software que permite crear un control de versiones de forma ordenada.

  Ramas (Branches): el comportamiento de un repositorio de Git es parecido a un árbol. Existe una rama original llamada "master"
    De master pueden salir ramas paralelas ("branches"), cada una con su nombre.
    
  Commits: son los puntos de guardado en una rama.
  
  Merge: es cuano una rama se fusiona con otra, creando una version que incluye los cambios en ambas. Lo normal y recomendable es fusionar las ramas con con la rama de la que han partido, pero no es obligatorio.
  
  Merge request: petición para fusionar una rama con otra, en la cual se comparan ambas versiones y se depuran errores e      incompatibilidades.
  
  Compare: compara el contenido de dos "commits" y de sus archivos.
    Esta opción es obligatoria en una "merge request" (peticion de fusion), pero que puede realizarse en cualquier momento.
  
  
  
  Pull: traer desde el repositorio al ordenador personal.
  
  Push: enviar datos al repositorio desde el ordenador personal.
  
  ```
  Rama_2:           C---C---C---C---C
                   /                 \ (merge request)
  Master: C---C---C---C---C---C---C---C---(...)
               \                 / (merge request)
  Rama_1:       C---C---C---C---C
  
  C: commit
  ```
  
