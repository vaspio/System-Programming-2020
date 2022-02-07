	K24: Προγραμματισμός Συστήματος - Εργασία 3

sdi1700121 - Πάσιος Βασίλειος

Ενδεικτική εκτέλεση:
  $ ​make
  $​ ./whoServer -q 5000 -s 2500 -w 4 -b 50	(Συνδεμένος στο linux10.di.uoa.gr της σχολής)
  $ ./master -w 4 -b 50 -s linux10.di.uoa.gr -p 2500 -i Database
  $ ./whoClient -q queryFile.txt -w 4 -sp 5000 -sip linux10.di.uoa.gr

  Στα αρχεία υπάρχουν μερικά σχόλια για την κατανόησή τους καθώς και μια μικρή περιγραφή πάνω από κάθε ορισμό συνάρτησης.
  Υλοποιούνται όλα τα ζητούμενα της εκφώνησης.

Έχουν υλοποιηθεί 3 προγράμματα: 1)ένα master πρόγραμμα που θα δημιουργεί μια σειρά από Worker διεργασίες,
2)έναν multithreaded whoServer που θα συλλέγει μέσω δικτύου summary statistics από τα Worker processes και ερωτήματα από πελάτες και 
3)ένα multithreaded client πρόγραμμα whoClient που θα δημιουργεί πολλά νήματα, όπου το κάθε νήμα παίζει το ρόλο ενός πελάτη που θα στέλνει ερωτήματα στον whoServer.

Ακριβής επεξήγηση της λειτουργίας των προγραμμάτων μπορεί να βρεθεί εδώ: http://cgi.di.uoa.gr/~antoulas/k24/homeworks/hw3-spring-2020.pdf

Διευκρινήσεις: Τα summary statistics δεν τυπώνονται αλλά βρίσκονται στον κώδικα σε μορφή σχολίου για όποιον χρειαστεί να τα τυπώσει.
Επίσης για το σωστό συγχρονισμό έχουν χρησιμοποιηθεί pthread_mutex_t και pthread_cond_t μεταβλητές.

Για valgrind(Memory Report): 
- whoServer: Τερματίζει με ^C οπότε αφήνει πίσω του λίγη αναγκαία μνήμη αλλά μέχρι εκεί γίνονται όλες οι απαραίτητες free().
- master: Τερματίζει με το signal SIGINT (^C), το κάνει handle και αφήνει 0 errors και "All heap blocks were freed -- no leaks are possible".
- workers: Τερματίζει με ^C αλλά υπάρχει η επιλογή να δεχθεί από τον whoServer query "/exit" στο οποίο τερματίζουν όλοι οι workers ομαλά και 
	ελευθερώνουν όλη την απαραίτητη μνήμη, όπου βγάζει 0 errors και "All heap blocks were freed -- no leaks are possible".
- whoClient: Τερματίζει ομαλά αλλά έχει suppressed errors και μερικά memory leaks.