/* libmpdclient
   (c) 2003-2009 The Music Player Daemon Project
   This project's homepage is: http://www.musicpd.org

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <mpd/queue.h>
#include <mpd/send.h>
#include <mpd/recv.h>
#include <mpd/pair.h>
#include <mpd/response.h>
#include <mpd/song.h>
#include "internal.h"
#include "isend.h"
#include "run.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

bool
mpd_send_list_queue_meta(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "playlistinfo", NULL);
}

bool
mpd_send_list_queue_range_meta(struct mpd_connection *connection,
			       unsigned start, unsigned end)
{
	return mpd_send_range_command(connection, "playlistinfo", start, end);
}

bool
mpd_send_get_queue_song_pos(struct mpd_connection *connection, unsigned pos)
{
	return mpd_send_int_command(connection, "playlistinfo", pos);
}

struct mpd_song *
mpd_run_get_queue_song_pos(struct mpd_connection *connection, unsigned pos)
{
	struct mpd_song *song;

	if (!mpd_run_check(connection) ||
	    !mpd_send_get_queue_song_pos(connection, pos))
		return NULL;

	song = mpd_recv_song(connection);
	if (!mpd_response_finish(connection) && song != NULL) {
		mpd_song_free(song);
		return NULL;
	}

	return song;

}

bool
mpd_send_get_queue_song_id(struct mpd_connection *connection, unsigned id)
{
	return mpd_send_int_command(connection, "playlistid", id);
}

struct mpd_song *
mpd_run_get_queue_song_id(struct mpd_connection *connection, unsigned id)
{
	struct mpd_song *song;

	if (!mpd_run_check(connection) ||
	    !mpd_send_get_queue_song_id(connection, id))
		return NULL;

	song = mpd_recv_song(connection);
	if (!mpd_response_finish(connection) && song != NULL) {
		mpd_song_free(song);
		return NULL;
	}

	return song;

}

bool
mpd_send_queue_changes_meta(struct mpd_connection *connection,
			    unsigned version)
{
	return mpd_send_ll_command(connection, "plchanges", version);
}

bool
mpd_send_queue_changes_brief(struct mpd_connection *connection,
			     unsigned version)
{
	return mpd_send_ll_command(connection, "plchangesposid", version);
}

bool
mpd_recv_queue_change_brief(struct mpd_connection *connection,
			    unsigned *position_r, unsigned *id_r)
{
	struct mpd_pair *pair;

	pair = mpd_recv_pair_named(connection, "cpos");
	if (pair == NULL)
		return false;

	*position_r = atoi(pair->value);
	mpd_return_pair(connection, pair);

	pair = mpd_recv_pair_named(connection, "Id");
	if (pair == NULL) {
		mpd_return_pair(connection, pair);

		if (!mpd_error_is_defined(&connection->error)) {
			mpd_error_code(&connection->error,
				       MPD_ERROR_MALFORMED);
			mpd_error_message(&connection->error,
					  "No id received");
		}

		return false;
	}

	*id_r = atoi(pair->value);
	mpd_return_pair(connection, pair);

	return !mpd_error_is_defined(&connection->error);
}

bool
mpd_send_add(struct mpd_connection *connection, const char *file)
{
	return mpd_send_command(connection, "add", file, NULL);
}

bool
mpd_send_add_id(struct mpd_connection *connection, const char *file)
{
	return mpd_send_command(connection, "addid", file, NULL);
}

int
mpd_recv_song_id(struct mpd_connection *connection)
{
	struct mpd_pair *pair;
	int id = -1;

	pair = mpd_recv_pair_named(connection, "Id");
	if (pair != NULL) {
		id = atoi(pair->value);
		mpd_return_pair(connection, pair);
	}

	return id;
}

int
mpd_run_add_id(struct mpd_connection *connection, const char *file)
{
	int id;

	if (!mpd_run_check(connection))
		return -1;

	if (!mpd_send_add_id(connection, file))
		return -1;

	id = mpd_recv_song_id(connection);

	if (!mpd_response_finish(connection))
		id = -1;

	return id;
}

bool
mpd_send_delete(struct mpd_connection *connection, unsigned pos)
{
	return mpd_send_int_command(connection, "delete", pos);
}

bool
mpd_send_delete_id(struct mpd_connection *connection, unsigned id)
{
	return mpd_send_int_command(connection, "deleteid", id);
}

bool
mpd_send_shuffle(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "shuffle", NULL);
}

bool
mpd_run_shuffle(struct mpd_connection *connection)
{
	return mpd_run_check(connection) &&
		mpd_send_shuffle(connection) &&
		mpd_response_finish(connection);
}

bool
mpd_send_shuffle_range(struct mpd_connection *connection, unsigned start, unsigned end)
{
	return mpd_send_range_command(connection, "shuffle", start, end);
}

bool
mpd_run_shuffle_range(struct mpd_connection *connection,
		      unsigned start, unsigned end)
{
	return mpd_run_check(connection) &&
		mpd_send_shuffle_range(connection, start, end) &&
		mpd_response_finish(connection);
}

bool
mpd_send_clear(struct mpd_connection *connection)
{
	return mpd_send_command(connection, "clear", NULL);
}

bool
mpd_run_clear(struct mpd_connection *connection)
{
	return mpd_run_check(connection) &&
		mpd_send_clear(connection) &&
		mpd_response_finish(connection);
}

bool
mpd_send_move(struct mpd_connection *connection, unsigned from, unsigned to)
{
	return mpd_send_int2_command(connection, "move", from, to);
}

bool
mpd_run_move(struct mpd_connection *connection, unsigned from, unsigned to)
{
	return mpd_run_check(connection) &&
		mpd_send_move(connection, from, to) &&
		mpd_response_finish(connection);
}

bool
mpd_send_move_id(struct mpd_connection *connection, unsigned from, unsigned to)
{
	return mpd_send_int2_command(connection, "moveid", from, to);
}

bool
mpd_run_move_id(struct mpd_connection *connection, unsigned from, unsigned to)
{
	return mpd_run_check(connection) &&
		mpd_send_move_id(connection, from, to) &&
		mpd_response_finish(connection);
}

bool
mpd_send_move_range(struct mpd_connection *connection,
		    unsigned start, unsigned end, unsigned to)
{
	return mpd_send_range_u_command(connection, "move", start, end, to);
}

bool
mpd_run_move_range(struct mpd_connection *connection,
		   unsigned start, unsigned end, unsigned to)
{
	return mpd_run_check(connection) &&
		mpd_send_move_range(connection, start, end, to) &&
		mpd_response_finish(connection);
}

bool
mpd_send_swap(struct mpd_connection *connection, unsigned pos1, unsigned pos2)
{
	return mpd_send_int2_command(connection, "swap", pos1, pos2);
}

bool
mpd_run_swap(struct mpd_connection *connection, unsigned pos1, unsigned pos2)
{
	return mpd_run_check(connection) &&
		mpd_send_swap(connection, pos1, pos2) &&
		mpd_response_finish(connection);
}

bool
mpd_send_swap_id(struct mpd_connection *connection, unsigned id1, unsigned id2)
{
	return mpd_send_int2_command(connection, "swapid", id1, id2);
}

bool
mpd_run_swap_id(struct mpd_connection *connection, unsigned id1, unsigned id2)
{
	return mpd_run_check(connection) &&
		mpd_send_swap_id(connection, id1, id2) &&
		mpd_response_finish(connection);
}
