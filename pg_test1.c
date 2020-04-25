
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libpq-fe.h"

static int pg_wait_result(PGconn *conn)
{
	unsigned int sock = -1;
	fd_set		input_mask;
	struct timeval timeout;

	//usleep(10000); // 10msec

	sock = PQsocket(conn);
	if(sock == -1) {
		return 0;
	}

	for(;;) {
		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10 * 1000;	// 10msec

		if(select(sock + 1, &input_mask, (fd_set *)NULL, (fd_set *)NULL, &timeout) < 0) {
			return 0;
		}
		if(FD_ISSET(sock, &input_mask) == 1) break;
	}

	return 1;
}

void pg_clean_result(PGconn *conn)
{
        PGresult        *res = NULL;

        for(;;) {
                res = PQgetResult(conn);
                if(res == NULL) break;
                PQclear(res);
        }
}

void print_tuples(PGresult *res)
{
	int r, c;
	int ntuples = PQntuples(res);
	int nfields = PQnfields(res);

	for(c = 0; c < nfields; c++) {
		printf("%s,", PQfname(res, c));
	}
	printf("\n");

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

//	PQsetnonblocking(conn, 1);

	if(!PQsendQuery(conn, sql)) {
		fprintf(stderr, "SQL error: %s", PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}

	i = 0;
	for(;;) {
		pg_wait_result(conn);

		if(!PQconsumeInput(conn)) {
			fprintf(stderr, "PQconsumeInput error: %s", PQerrorMessage(conn));
			break;
		}
		if(!PQisBusy(conn)) break;
	}

	res = PQgetResult(conn);

	exec_status = PQresultStatus(res);
	fprintf(stderr, "exec_status: %d\n", exec_status);

	if(exec_status == PGRES_TUPLES_OK) {
		fprintf(stderr, "tuples ok \n");
		print_tuples(res);
	}
	PQclear(res);

	pg_clean_result(conn);

	PQfinish(conn);
	return 0;
}



