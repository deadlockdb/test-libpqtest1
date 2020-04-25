
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libpq-fe.h"

void pg_clean_result(PGconn *conn)
{
	PGresult	*res = NULL;

	for(;;) {
		res = PQgetResult(conn);
		if(res == NULL) break;
		PQclear(res);
	}
}

void print_colname_list(PGresult *res)
{
	int c;
	int nfields = PQnfields(res);

	for(c = 0; c < nfields; c++) {
		printf("%s,", PQfname(res, c));
	}
	printf("\n");
}

void print_tuples(PGresult *res)
{
	int r, c;
	int ntuples = PQntuples(res);
	int nfields = PQnfields(res);

	for(r = 0; r < ntuples; r++) {
		for(c = 0; c < nfields; c++) {
			printf("%s,", PQgetvalue(res, r, c));
		}
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	const char *conninfo;
	const char *sql;
	PGconn *conn;
	PGresult *res;
	ExecStatusType exec_status;
	int i;
	int first_flg = 1;

        if(argc > 1) {
		conninfo = argv[1];
	} else {
		conninfo = "";
	}
        if(argc > 2) {
		sql = argv[2];
	} else {
		sql = "select * from pg_database";
	}
	
	conn = PQconnectdb(conninfo);
	if(PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection error: %s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}

	if(!PQsendQuery(conn, sql)) {
		fprintf(stderr, "SQL error: %s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}
	if(!PQsetSingleRowMode(conn)) {
		fprintf(stderr, "SQL error: %s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}

	i = 0;
	for(;;) {
		res = PQgetResult(conn);
		if(res == NULL) break;

		if(first_flg) {
			print_colname_list(res);
			first_flg = 0;
		}

		if (PQresultStatus(res) == PGRES_SINGLE_TUPLE) {
			print_tuples(res);
			PQclear(res);
		} else {
			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				fprintf(stderr, "SQL error: %s", PQerrorMessage(conn));
				PQclear(res);
				PQfinish(conn);
				exit(1);
			}
			PQclear(res);
			break;
		}
	}

	pg_clean_result(conn);

	PQfinish(conn);
	return 0;
}



