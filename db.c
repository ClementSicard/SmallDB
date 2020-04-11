// This file requires at least C99 to compile

/**
 * @file   db.c
 * @author Merlin Nimier-David <merlin.nimier-david@epfl.ch>
 * @author Jean-Cédric Chappelier <jean-cedric.chappelier@epfl.ch>
 *
 * @copyright EPFL 2020
**/
/**
 * @section DESCRIPTION
 *
 * Template du homework du cours CS-207, année 2020.
**/

#include <stdio.h>
#include <stdlib.h>   // EXIT_SUCCESS/FAILURE
#include <math.h>     // fabs()
#include <string.h>   // memset()
#include <stdint.h>   // uint32_t
#include <inttypes.h> // PRIu32 & SCNu32

// ----------------------------------------------
//   ___             _            _
//  / __|___ _ _  __| |_ __ _ _ _| |_ ___
// | (__/ _ \ ' \(_-<  _/ _` | ' \  _(_-<
//  \___\___/_||_/__/\__\__,_|_||_\__/__/

#define DB_MAX_SIZE 20u
#define QUERY_MAX_SIZE 5u

// ----------------------------------------------
//  _____
// |_   _|  _ _ __  ___ ___
//   | || || | '_ \/ -_|_-<
//   |_| \_, | .__/\___/__/
//       |__/|_|

/* Définissez ici les types demandés :
 *    StudentKind,
 *    SCIPER,
 *    Student,
 *    Database,
 * et QueryResult.
 */

typedef enum
{
    Bachelor,
    Master,
    Exchange,
    StudentKindCount
} StudentKind;
// BA -> 0, MA -> 1, Ex -> 2

typedef unsigned int SCIPER;

typedef struct Student Student;

struct Student
{
    SCIPER sciper;
    double grade_sn, grade_hw, grade_exam;
    StudentKind type;
    Student const *teammate;
};

typedef Student Database[DB_MAX_SIZE];

typedef Student const *QueryResult[QUERY_MAX_SIZE];

// ----------------------------------------------
//   ___               _
//  / _ \ _  _ ___ _ _(_)___ ___
// | (_) | || / -_) '_| / -_|_-<
//  \__\_\\_,_\___|_| |_\___/__/

size_t db_entry_count(const Database db)
{
    for (size_t i = 0; i < DB_MAX_SIZE; ++i)
    {
        if (db[i].sciper == 0)
        {
            return i;
        }
    }
    return DB_MAX_SIZE;
}

// ----------------------------------------------
const Student *get_student_by_sciper(const Database db, SCIPER sciper)
{
    const size_t s = db_entry_count(db);
    for (size_t t = 0; t < s; ++t)
    {
        if (db[t].sciper == sciper)
        {
            return &db[t];
        }
    }
    return NULL;
}

// ----------------------------------------------
int check_teammates_consistency(const Database db)
{
    const size_t end = db_entry_count(db);
    for (size_t i = 0; i < end; ++i)
    {
        Student const *tm = db[i].teammate;
        if (tm != NULL)
        {
            Student const *tm_of_tm = get_student_by_sciper(db, tm->sciper)->teammate;
            if (tm_of_tm == NULL)
            {
                fprintf(stderr, "%" SCNu32 " a %" SCNu32 " comme binôme mais %" SCNu32 " n'a pas de binôme.\n", db[i].sciper, tm->sciper, tm->sciper);
                return -1;
            }
            else if (tm_of_tm != &db[i])
            {
                fprintf(stderr, "%" SCNu32 " a %" SCNu32 " comme binôme mais %" SCNu32 " a %" SCNu32 " comme binôme.\n", db[i].sciper, tm->sciper, tm->sciper, tm_of_tm->sciper);
                return -1;
            }
        }
    }
    return 0;
}

// ----------------------------------------------
void get_students_by_type(const Database db, StudentKind type, QueryResult result_out)
{
    memset(result_out, 0, QUERY_MAX_SIZE * sizeof(Student *));
    const size_t end = db_entry_count(db);
    size_t count = 0, i = 0;

    while (count < QUERY_MAX_SIZE && i < end)
    {
        if (db[i].type == type)
        {
            result_out[count] = &db[i];
            ++count;
        }
        ++i;
    }
}

// ----------------------------------------------
double grade_average(const Student *stud)
{
    if (stud == NULL)
        return 0.0;
    return 0.1 * stud->grade_hw + 0.4 * stud->grade_sn + 0.5 * stud->grade_exam;
}

// ----------------------------------------------
double team_diff(const Student *stud)
{
    return fabs(grade_average(stud) - grade_average(stud->teammate));
}

// ----------------------------------------------
#define student_to_index(tab, student) (size_t)((student) - (tab))

// ----------------------------------------------
int contains(Database db, Student s)
{
    const size_t end = db_entry_count(db);
    for (size_t i = 0; i < end; ++i)
    {
        if (db[i].sciper == s.sciper || db[i].sciper == (s.teammate)->sciper)
        {
            return 1;
        }
    }
    return 0;
}

#define diff_max 6.0

size_t diff_index(QueryResult qr)
{
    double min = diff_max;
    size_t idx = 0;
    for (size_t i = 0; i < QUERY_MAX_SIZE; ++i)
    {
        double diff = team_diff(qr[i]);
        if (diff < min)
        {
            idx = i;
            min = diff;
        }
    }
    return idx;
}

void get_least_homegenous_teams(const Database db, QueryResult result_out)
{
    memset(result_out, 0, QUERY_MAX_SIZE * sizeof(Student *));
    const size_t end = db_entry_count(db);
    // Créer une copie de la db avec uniquement les binômes
    Database db2;
    memset(db2, 0, sizeof(db2));

    size_t count = 0;
    for (size_t i = 0; i < end; ++i)
    {
        if (db[i].teammate != NULL)
        {
            if (!(contains(db2, db[i])))
            {
                if (grade_average(db[i].teammate) < grade_average(&db[i]))
                {
                    db2[count] = *db[i].teammate;
                    ++count;
                }
                else
                {
                    db2[count] = db[i];
                    ++count;
                }
            }
        }
    }
    count = 0;
    const size_t db2_size = db_entry_count(db2);
    for (size_t i = 0; i < QUERY_MAX_SIZE && i < db2_size; ++i)
    {
        result_out[i] = &db2[i];
        ++count;
    }

    while (count < db2_size)
    {
        size_t sd_idx = diff_index(result_out);
        double smallest_diff = team_diff(result_out[sd_idx]);

        if (team_diff(&db2[count]) > smallest_diff)
        {
            result_out[sd_idx] = &db2[count];
        }
        ++count;
    }
}

// ----------------------------------------------
//  ___   _____
// |_ _| / / _ \
//  | | / / (_) |
// |___/_/ \___/

int load_database(Database db_out, const char *filename)
{
    FILE *entree = NULL;
    size_t count = 0;

    for (size_t i = 0; i < DB_MAX_SIZE; ++i)
    {
        db_out[i].sciper = 0;
    }

    entree = fopen(filename, "r");
    if (entree == NULL)
    {
        fprintf(stderr, "Erreur : impossible de lire le fichier %s.\n", filename);
        return -1;
    }

    SCIPER teammates[DB_MAX_SIZE];
    memset(teammates, 0, sizeof(teammates));
    do
    {
        Student stud;
        memset(&stud, 0, sizeof(Student));

        // Récupère le SCIPER
        if (fscanf(entree, "%" SCNu32, &stud.sciper) != 1)
        {
            if (feof(entree))
            {
                break;
            }
            fprintf(stderr, "Erreur de lecture du SCIPER\n");
            return -1;
        }

        // Récupère les notes
        if (fscanf(entree, "%lf %lf %lf", &stud.grade_sn, &stud.grade_hw, &stud.grade_exam) != 3)
        {
            fprintf(stderr, "Erreur de lecture des notes\n");
            return -1;
        }

        // Récupère le type
        int tmp_kind;
        if (fscanf(entree, "%d", &tmp_kind) != 1)
        {
            fprintf(stderr, "Erreur de lecture du type d'étudiant\n");
            return -1;
        }
        if (tmp_kind < 0 || tmp_kind >= StudentKindCount)
        {
            fprintf(stderr, "Erreur : type d'étudiant non-reconnu\n");
            return -1;
        }
        else
        {
            stud.type = tmp_kind;
        }
        // Récupère le teammate et le stock dans le tableau
        if (fscanf(entree, "%" SCNu32, &teammates[count]) != 1)
        {
            fprintf(stderr, "Erreur de lecture du SCIPER du partenaire\n");
            return -1;
        }
        db_out[count] = stud;
        ++count;
    } while (!feof(entree) && count < DB_MAX_SIZE);

    fclose(entree);
    const size_t end = db_entry_count(db_out);
    for (size_t i = 0; i < end; ++i)
    {
        db_out[i].teammate = get_student_by_sciper(db_out, teammates[i]);
    }
    // check for consistency of pointers (pairs)

    return check_teammates_consistency(db_out);
}

// ----------------------------------------------
void fprintf_student_kind(FILE *restrict stream, StudentKind sk)
{
    switch (sk)
    {
    case Bachelor:
        fputs("bachelor", stream);
        break;
    case Master:
        fputs("master  ", stream);
        break;
    case Exchange:
        fputs("exchange", stream);
        break;
    default:
        fputs("unknown ", stream);
        break;
    }
}

// ----------------------------------------------
void write_student(const Student *student, FILE *fp)
{
    fprintf(fp, "%07d - %.2f, %.2f, %.2f - ",
            student->sciper, student->grade_sn, student->grade_hw,
            student->grade_exam);
    fprintf_student_kind(fp, student->type);
    if (student->teammate != NULL)
    {
        fprintf(fp, " - %06" PRIu32, student->teammate->sciper);
    }
    else
    {
        fprintf(fp, " - none");
    }
    fprintf(fp, "\n");
}

// ----------------------------------------------
int write_query_results(QueryResult result, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file %s for writting: ", filename);
        perror(NULL); // optionnel
        return -1;
    }
    for (size_t i = 0; i < QUERY_MAX_SIZE && result[i] != NULL; ++i)
    {
        write_student(result[i], fp);
    }
    fclose(fp);
    printf("Query results saved to: %s\n", filename);
    return 0;
}

// ----------------------------------------------
//  __  __      _
// |  \/  |__ _(_)_ _
// | |\/| / _` | | ' \
// |_|  |_\__,_|_|_||_|

int main(int argc, char **argv)
{
    const char *input_filename = "many_teams.txt"; // default input filename
    if (argc >= 2)
    {
        input_filename = argv[1];
    }

    Database db;
    memset(db, 0, sizeof(db));
    int success = load_database(db, input_filename);

    if (success != 0)
    {
        fputs("Could not load database.\n", stderr);
        return EXIT_FAILURE;
    }

    // Print contents of database
    puts("+ ----------------- +\n"
         "| Students database |\n"
         "+ ----------------- +");
    const size_t end = db_entry_count(db);
    for (size_t i = 0; i < end; ++i)
    {
        write_student(&db[i], stdout);
    }

    // Extract students of each kind
    QueryResult res;
    memset(res, 0, sizeof(res));
    char filename[] = "res_type_00.txt";
    const size_t filename_mem_size = strlen(filename) + 1;
    for (StudentKind sk = Bachelor; sk < StudentKindCount; ++sk)
    {
        get_students_by_type(db, sk, res);
        snprintf(filename, filename_mem_size, "res_type_%02d.txt", sk);
        write_query_results(res, filename);
    }

    // Extract least homogeneous teams
    get_least_homegenous_teams(db, res);
    write_query_results(res, "bad_teams.txt");

    return EXIT_SUCCESS;
}
