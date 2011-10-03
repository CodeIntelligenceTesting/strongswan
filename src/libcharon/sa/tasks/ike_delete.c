/*
 * Copyright (C) 2006-2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "ike_delete.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>


typedef struct private_ike_delete_t private_ike_delete_t;

/**
 * Private members of a ike_delete_t task.
 */
struct private_ike_delete_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_delete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * are we deleting a rekeyed SA?
	 */
	bool rekeyed;

	/**
	 * are we responding to a delete, but have initated our own?
	 */
	bool simultaneous;
};

METHOD(task_t, build_i, status_t,
	private_ike_delete_t *this, message_t *message)
{
	delete_payload_t *delete_payload;

	DBG0(DBG_IKE, "deleting IKE_SA %s[%d] between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));

	delete_payload = delete_payload_create(PROTO_IKE);
	message->add_payload(message, (payload_t*)delete_payload);

	if (this->ike_sa->get_state(this->ike_sa) == IKE_REKEYING)
	{
		this->rekeyed = TRUE;
	}
	this->ike_sa->set_state(this->ike_sa, IKE_DELETING);

	DBG1(DBG_IKE, "sending DELETE for IKE_SA %s[%d]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa));

	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_ike_delete_t *this, message_t *message)
{
	DBG0(DBG_IKE, "IKE_SA deleted");
	if (!this->rekeyed)
	{	/* invoke ike_down() hook if SA has not been rekeyed */
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
	}
	/* completed, delete IKE_SA by returning DESTROY_ME */
	return DESTROY_ME;
}

METHOD(task_t, process_r, status_t,
	private_ike_delete_t *this, message_t *message)
{
	/* we don't even scan the payloads, as the message wouldn't have
	 * come so far without being correct */
	DBG1(DBG_IKE, "received DELETE for IKE_SA %s[%d]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa));
	DBG0(DBG_IKE, "deleting IKE_SA %s[%d] between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));

	switch (this->ike_sa->get_state(this->ike_sa))
	{
		case IKE_ESTABLISHED:
			this->ike_sa->set_state(this->ike_sa, IKE_DELETING);
			this->ike_sa->reestablish(this->ike_sa);
			return NEED_MORE;
		case IKE_REKEYING:
			this->rekeyed = TRUE;
			break;
		case IKE_DELETING:
			this->simultaneous = TRUE;
			break;
		default:
			break;
	}
	this->ike_sa->set_state(this->ike_sa, IKE_DELETING);
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_delete_t *this, message_t *message)
{
	DBG0(DBG_IKE, "IKE_SA deleted");

	if (this->simultaneous)
	{
		/* wait for peer's response for our delete request, but set a timeout */
		return SUCCESS;
	}
	if (!this->rekeyed)
	{	/* invoke ike_down() hook if SA has not been rekeyed */
		charon->bus->ike_updown(charon->bus, this->ike_sa, FALSE);
	}
	/* completed, delete IKE_SA by returning DESTROY_ME */
	return DESTROY_ME;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_delete_t *this)
{
	return IKE_DELETE;
}

METHOD(task_t, migrate, void,
	private_ike_delete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->simultaneous = FALSE;
}

METHOD(task_t, destroy, void,
	private_ike_delete_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_delete_t *ike_delete_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_delete_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
