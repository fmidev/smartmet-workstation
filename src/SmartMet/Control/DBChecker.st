// Pilvisyyskorjaus sateen perusteella
// Jos sataa, pilveä on vähintään 50 %
IF(RR > 0.01 AND N < 50) {
  N = 50
}

// Lisätään ala- tai keskipilviä niin että jompi kumpi on sama kuin kokonaispilvisyys.
// Valinta tehdään sen mukaan kumpaa on jo alunperin enemmän.
IF (CL < N AND CL >= CM) {
  CL = N
}
//ja toisinpäin
IF (CM < N AND CL < CM) {
  CM = N
}

// Vähennetään ala- ja keskipilviä, jos ne ovat suurempia kuin kokonaispilvisyys.
IF (CL > N) {
  CL = N
}
IF (CM > N) {
  CM = N
}

// Sateen tyyppi
// Laitetaan kuuroksi, jos puuttuu.
IF((PRET != 1 AND PRET != 2) OR PRET == MISS) {
  PRET = 2
}

// Sateen olomuoto
// Laitetaan vedeksi, jos puuttuu.
IF(PREF == MISS) {
  PREF = 1
}

//Tihkulle, jäätävälle tihkulle sekä jäätävälle sateelle sateen tyyppi jatkuvaksi
IF((PREF == 0 OR PREF == 4 OR PREF == 5) AND PRET != 1) {
  PRET = 1
}

// Säteilykorjaus
SRAD  = (1 - 0.67 * (N/100) ^ 3.32) / (1 - 0.67 * (N_ORIG/100) ^ 3.32) * SRAD_ORIG
LRAD  = (1 + 0.22 * (N/100) ^ 2.75) / (1 + 0.22 * (N_ORIG/100) ^ 2.75) * LRAD_ORIG

IF((SRAD > 0) AND (EANGLE <= 0)) {
  SRAD = 0  
}

IF( SRAD < 0 ){ 
  SRAD = 0 
}

// Luo wsmax:n loppuun jos se on puuttuvaa
IF(FHOUR > 203 AND WSMAX == MISS) {
 WSMAX = WS * 1.1
}

//Pyöristää pref:n, pret:n ja sumun kokonaisluvuiksi.
PREF = floor(PREF)
PRET = floor(PRET)
FOG = floor(FOG)