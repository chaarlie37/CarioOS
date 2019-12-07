PROGRAM practicaarrays;

TYPE

  tabla_datos = ARRAY[1..6, 1..6] OF integer;
  array_meses = ARRAY[1..6] OF string;
  array_conceptos = ARRAY[1..6] OF string;

VAR

  tabla:tabla_datos;
  conceptos:array_conceptos;
  meses:array_meses;
  m,c:integer;
  dato:integer;
  mes, concepto:integer;


  PROCEDURE mostrarGastoConceptoMes(m,c:integer);
  BEGIN
    writeln('El gasto de ', conceptos[c], ' en ', meses[m], ' es ', tabla[m, c]);
  END;

  PROCEDURE mostrarGastoPorMeses;
  VAR
    suma:integer;
    i:integer;
  BEGIN
    FOR m := 1 TO 6 DO BEGIN
        FOR c := 1 TO 6 DO BEGIN
          suma := suma + tabla[m,c];

        END;
        writeln('Gastos de ', meses[m],': ', suma);
      END;
  END;

  PROCEDURE mostrarGastoPorConceptos;
  VAR
    suma:integer;
    i:integer;
  BEGIN
    FOR c := 1 TO 6 DO BEGIN
        FOR m := 1 TO 6 DO BEGIN
          suma := suma + tabla[m,c];
        END;
        writeln('Gastos de ', conceptos[c],': ', suma);
      END;
  END;

  PROCEDURE mostrarGastoTotal;
  VAR
    suma:integer;
    c,m:integer;
  BEGIN
    suma := 0;
    FOR m := 1 TO 6 DO
      FOR c := 1 TO 6 DO
        suma := suma + tabla[c,m];
    writeln('El gasto total es de ', suma, '.000 euros.');
  END;

BEGIN


  conceptos[1]:='Textil';
  conceptos[2]:='Automovil';
  conceptos[3]:='Hogar';
  conceptos[4]:='Jardineria';
  conceptos[5]:='Ferreteria';
  conceptos[6]:='Jugueteria';


  meses[1]:='Enero';
  meses[2]:='Febrero';
  meses[3]:='Marzo';
  meses[4]:='Abril';
  meses[5]:='Mayo';
  meses[6]:='Junio';

  FOR m := 1 TO 6 DO BEGIN
    writeln('Introduzca los datos del mes ', meses[m]);
    FOR c := 1 TO 6 DO BEGIN
      write(conceptos[c], ': ');
      readln(tabla[m,c]);
    END;
    writeln;
  END;

  writeln;

  mostrarGastoPorMeses;
  mostrarGastoPorConceptos;
  mostrarGastoTotal;

  write('Mes (como numero) :');
  readln(mes);
  writeln('Pulse el numero del concepto que desee');
  writeln('1: Textil''2:Automovil''3:Hogar''4:Jardineria''5:Ferretería''6:Jugueteria');
  read(concepto);

  mostrarGastoConceptoMes(mes,concepto);
  readln();
END.
