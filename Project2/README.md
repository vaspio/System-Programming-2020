	K24: Προγραμματισμός Συστήματος - Εργασία 2

sdi1700121 - Πάσιος Βασίλειος

Ενδεικτική εκτέλεση:
  $ ./create_infiles.sh Countries.txt Diseases.txt Database 5 5
  $ ​make
  $​ ./diseaseAggregator -w 4 -b 50 -i Database

  Στα αρχεία υπάρχουν μερικά σχόλια για την κατανόησή τους καθώς και μια μικρή περιγραφή πάνω από κάθε ορισμό συνάρτησης. 
  Η εργασία υλοποιεί όλα τα ζητούμενα της εκφώνησης και τερματίζει(εξαιρεση το SIGKILL) με "All heap blocks were freed -- no leaks are possible" με την χρήση valgrind.

  Το bash script create_infiles.sh δημιουργεί έναν κατάλογο(Database) στον οποίο δημιουργεί subdirectories, ένα για το όνομα κάθε χώρας μέσα στο Countries.txt(1ο όρισμα). Σε κάθε χώρα δημιουργεί numFilesPerDirectory(4ο ορισμα) αρχεία με όνομα μιας τυχαίας ημερομηνίας. Σε κάθε αρχείο δημιουργεί numRecordsPerFile(5ο) patient records μορφής: 889 ENTER Mary Smith COVID-2019 23.
  Tα ids δημιουργούνται σειριακά και τα "ENTER" ή "EXIT" και τα ονόματα δημιουργούνται τυχαία(τα ονόματα έχουν κεφαλαίο το πρώτο γράμμα).

  Ο diseaseAggregator(main.c) δέχεται τα ορίσματα και δημιουργεί k workers(k ο αριθμός μετα το -w) καθώς και 2 pipes για τον καθένα(ένα για read και ένα για write). Μετά μέσω των pipes διαβάζοντας το Database χωρίζει τις χώρες και τις αναθέτει στον κάθε worker.Tώρα ο κάθε worker παίρνει ότι αρχεία του έχουν ανατεθεί και τα βάζει σε δομές(list,hashtables).
    
  Να σημειωθεί ότι το πρόγραμμα δεν τυπώνει επίτηδες "ERROR" για λόγους καλύτερης ορατότητας του κώδικα. !!! 

  Μετά στέλνει για κάθε αρχείο τα Summary Statistics του στον diseaseAggregator ο οποίος τα τυπώνει.
  Ο diseaseAggregator δέχεται τα Statistics με την χρήση της select() για να μην περιμένουν όλοι οι Workers κάποιον αργό worker.

  Αφού γίνει αυτό ο diseaseAggregator στέλνει ένα σήμα SIGUSR2 σε όλους τους workers για να είναι έτοιμοι για τα queries.   

  Στα queries έχει προστεθεί το "/intquitWorker n_worker" στο οποίο αν δωθεί n_worker(αριθμός κάποιου worker) τερματίζει τον worker αυτό και στην θέση του φτιάχνει έναν καινουργιο worker ο οποίος ξαναφτιάχνει τις δομές του, τυπώνει τα statistics του και τον αντικαθιστά. Αν δωθεί n_worker = με τον αριθμό των workers τότε τερματίζει όλους τους workers κάνοντας όλα τα απαραίτητα free();

  Επίσης αν δωθεί SIGUSR1 από τον diseaseAggregator τότε οι workers ψάχνουν για νέα αρχεία. Αν βρουν, τότε προσθέτουν όλα τα valid records στις δομές τους και στέλνουν Summary Statistics για αυτά(Έχει τεσταριστεί με sleep και SignalSend() αφού τοποθετήσω καινούργια αρχεία σε καποια χώρα).

  Σε περίπτωση SIGINT ή SIGQUIT προς τους workers, οι workers κάνουν free όλη την απαραίτητη μνήμη και δημιουργούν αρχέια log_file.xxx(xxx=pid) τα οποία περιέχουν τις χώρες που είχαν αναλάβει και statistics (success,error).

  O diseaseAggregator όταν είναι έτοιμος να τελειώσει στέλνει SIGKILL σε όλους τους workers, απελευθερώνει την μνήμη και δημιουργεί ένα logfile με όλες τις χώρες και τα στατιστικα fail,success.

 



  