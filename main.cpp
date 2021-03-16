/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lucas <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/09 12:23:41 by lucas             #+#    #+#             */
/*   Updated: 2021/03/15 18:02:14 by lucas            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/IRCserv.hpp"
#include <stdio.h>
#define PORT 23

void		sig_handler(int signal)
{
	if (signal == SIGINT)
	{
		for (size_t i = 0; i < g_cli_sock.size(); i++)
			closesocket(g_cli_sock[i]);
		closesocket(g_serv_sock);
		exit(0);
	}
}

int main(void)
{
	SOCKADDR_IN sin;
	socklen_t recsize = sizeof(sin);

	SOCKADDR_IN csin;
	socklen_t crecsize = sizeof(csin);

	signal(SIGINT, sig_handler);
	int sock_err;

	g_serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	if (g_serv_sock == INVALID_SOCKET)
	{
		perror("socket");
		return (1);
	}
	std::cout << "La socket %d " << g_serv_sock << " est maintenant ouverte en mode TCP/IP\n";

	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sock_err = bind(g_serv_sock, (SOCKADDR*)&sin, recsize);

	if (sock_err == SOCKET_ERROR)
	{
		perror("bind");
		return (1);
	}
	sock_err = listen(g_serv_sock, 5);
	std::cout << "Listage du port %d" << PORT << "...\n";

	if (sock_err == SOCKET_ERROR)
	{
		perror("listen");
		return (1);
	}

	std::cout << "Patientez pendant que le client se connecte sur le port %d " << PORT << "...\n";

	char			entry[256];
	char			buf[256];
	t_select		sel;
	int				nb_client;

	strcpy(&entry[0], "Salut a toi qui vient de te connecter");
	bzero(buf, 256);
	nb_client = 0;
	while (1)
	{
		if (g_cli_sock.size() < 6)
		{
			FD_ZERO(&sel.fd_s);
			FD_SET(g_serv_sock, &sel.fd_s);
			sel.timeout.tv_sec = 0;
			sel.timeout.tv_usec = 10;
			if ((sel.res = select(8, &sel.fd_s, NULL, &sel.fd_s, &sel.timeout)) > 0)
			{
				g_cli_sock.push_back(accept(g_serv_sock, (SOCKADDR*)&csin, &crecsize));
				std::cout << "Un client se connecte avec la socket" <<
				g_cli_sock.back() << " de " << inet_ntoa(csin.sin_addr) <<
				":" << htons(csin.sin_port) << std::endl;
				sock_err = send(g_cli_sock.back(), entry, 39, 0);
				nb_client++;
			}
		}
		size_t		i = 0;
		while (i < g_cli_sock.size())
		{
			bzero(buf, 256);
			FD_ZERO(&sel.fd_s);
			FD_SET(g_cli_sock[i], &sel.fd_s);
			sel.timeout.tv_sec = 0;
			sel.timeout.tv_usec = 10;
			sel.res = select(8, &sel.fd_s, NULL, NULL, &sel.timeout);
			if (sel.res > 0)
			{
				if ((sock_err = recv(g_cli_sock[i], buf, 255, 0)) == SOCKET_ERROR)
				{
					std::cout << "Erreur transmission\n";
					break ;
				}
				buf[sock_err] = '\0';
				if (buf[0] != '\0')
				{
					std::cout << buf << std::endl;
					size_t k = 0;
					while (k < 2)
					{
						if (k != i)
							send(g_cli_sock[k], buf, strlen(buf) + 1, 0);
						++k;
					}
				}
			}
			i++;
		}
	}
	/* Fermeture de la socket client et de la socket serveur */
	std::cout << "Fermeture de la socket client\n";
	for (int i = 0; i < nb_client; i++)
		closesocket(g_cli_sock[i]);
	std::cout << "Fermeture de la socket serveur\n";
	closesocket(g_serv_sock);
	std::cout << "Fermeture du serveur terminée\n";

	return (0);
}
