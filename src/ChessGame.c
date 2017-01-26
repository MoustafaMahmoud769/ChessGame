#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

void printBoard();
int tolower(int c);
int toupper(int c);
void setGame();
void applyMove(int player);
int isdigit(int c);

struct Peace {
	char symbol;
	int team;
} grid[8][8];
int startX, startY, endX, endY, moves;
char now, dead[2][16], update[60];
int ptr[2], history[4][1000000], limit, curr, k;

void save() {
	FILE *fp;
	fp = fopen("data.txt", "w+");
	int i, j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			fprintf(fp, "%c %d\n", grid[i][j].symbol, grid[i][j].team);
		}
	}
	fprintf(fp, "%d %d %d \n", moves, ptr[0], ptr[1]);
	for (i = 0; i < ptr[0]; ++i) {
		fprintf(fp, "%c ", dead[0][i]);
	}
	fprintf(fp, "\n");
	for (i = 0; i < ptr[1]; ++i) {
		fprintf(fp, "%c ", dead[1][i]);
	}
	fprintf(fp, "\n");
	fclose(fp);
	printf("Saved successfully.\n");
}

void setNumbers(char buff[]) {
	int i = 0, j = 0, val = 0, arr[5];
	while (buff[i]) {
		if (isdigit(buff[i])) {
			val *= 10;
			val += buff[i] - '0';
		} else {
			arr[j++] = val;
			val = 0;
		}
		++i;
	}
	moves = arr[0], ptr[0] = arr[1], ptr[1] = arr[2];
}

void load() {
	char buff[1000];
	FILE *fp;
	fp = fopen("data.txt", "r");
	int i, j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			fgets(buff, 255, (FILE*) fp);
			grid[i][j].symbol = buff[0];
			grid[i][j].team = buff[2] - '0';
		}
	}
	fgets(buff, 255, (FILE*) fp);
	setNumbers(buff);
	fgets(buff, 255, (FILE*) fp);
	j = 0;
	for (i = 0; i < ptr[0]; ++i) {
		dead[0][i] = buff[j];
		j += 2;
	}
	fgets(buff, 255, (FILE*) fp);
	j = 0;
	for (i = 0; i < ptr[1]; ++i) {
		dead[1][i] = buff[j];
		j += 2;
	}
	fclose(fp);
	limit = 0;
	curr = 0;
	printBoard();

}

void reset() {
	int i, j, player = 1;
	for (i = 0; i < curr; ++i) {
		startX = history[0][i], startY = history[1][i];
		endX = history[2][i], endY = history[3][i];
		applyMove(player);
		for (j = 0; j < 8; ++j) {
			if (grid[7][j].symbol == 'p')
				grid[7][j].symbol = update[k++];
			if (grid[0][j].symbol == 'P')
				grid[0][j].symbol = update[k++];
		}
		player = 3 - player;
		moves++;
	}
}

void undo() {
	if (curr == 0) {
		printf("Can't Undo \n");
		return;
	}
	curr--;
	setGame();
	reset();
	printBoard();
}

void redo() {
	if (curr == limit) {
		printf("Can't Redo \n");
		return;
	}
	curr++;
	setGame();
	reset();
	printBoard();
}

int max(int x, int y) {
	return (x > y ? x : y);
}

int inRange(int x) {
	return (x >= 0 && x <= 7);
}

int isValidInput(char input[]) {
	startY = tolower(input[0]) - 'a';
	endY = tolower(input[2]) - 'a';
	startX = tolower(input[1]) - '1';
	endX = tolower(input[3]) - '1';
	char nullTerminator = 0;
	return inRange(startX) && inRange(endX) && inRange(startY) && inRange(endY)
			&& input[4] == nullTerminator;
}

void findKing(int player, int *x, int *y) {
	int i, j;
	char king = (player == 1 ? 'k' : 'K');
	for (i = 0; i < 8; ++i)
		for (j = 0; j < 8; ++j)
			if (grid[i][j].symbol == king) {
				*x = i, *y = j;
				return;
			}
}

int checkDirection(int player, int x, int y, int xStep, int yStep,
		int isDiagonal) {
	x += xStep, y += yStep;
	int steps = 1;
	while (inRange(x) && inRange(y)) {
		if (grid[x][y].team == 3 - player) {
			char danger = tolower(grid[x][y].symbol);
			if (isDiagonal) {
				if (danger == 'b' || danger == 'q')
					return 1;
				else if (danger == 'p' && steps == 1)
					return 1;
				else
					return 0;
			} else {
				if (danger == 'r' || danger == 'q')
					return 1;
				else
					return 0;
			}
		}
		if (grid[x][y].team == player)
			return 0;
		x += xStep, y += yStep, steps++;
	}
	return 0;
}

int isKingInDanger(int player) {
	int x, y, i;
	findKing(player, &x, &y);
	int dx[] = { 0, 0, 1, -1, 1, 1, -1, -1 };
	int dy[] = { 1, -1, 0, 0, 1, -1, 1, -1 };
	int dx2[] = { 1, 1, -1, -1, 2, 2, -2, -2 };
	int dy2[] = { 2, -2, 2, -2, 1, -1, 1, -1 };
	int isDiagonal[] = { 0, 0, 0, 0, 1, 1, 1, 1 };
	for (i = 0; i < 8; ++i) {
		if (checkDirection(player, x, y, dx[i], dy[i], isDiagonal[i]))
			return 1;
		if (inRange(x + dx2[i]) && inRange(y + dy2[i])) {
			if (grid[x + dx2[i]][y + dy2[i]].team == 3 - player
					&& tolower(grid[x + dx2[i]][y + dy2[i]].symbol) == 'n')
				return 1;
		}
	}
	return 0;
}

void applyMove(int player) {
	now = grid[endX][endY].symbol;
	grid[endX][endY].symbol = grid[startX][startY].symbol;
	grid[endX][endY].team = grid[startX][startY].team;
	char emptyCell = ((startX + startY) & 1 ? '*' : '#');
	grid[startX][startY].team = 0;
	grid[startX][startY].symbol = emptyCell;
	if (now != '*' && now != '#') {
		dead[2 - player][ptr[2 - player]++] = now;
	}
}

void applyBack(int player) {
	grid[startX][startY].team = grid[endX][endY].team;
	grid[startX][startY].symbol = grid[endX][endY].symbol;
	grid[endX][endY].symbol = now;
	if (now != '*' && now != '#') {
		grid[endX][endY].team = 3 - grid[endX][endY].team;
		ptr[2 - player]--;
	} else
		grid[endX][endY].team = 0;
}

int checkSelfDanger(int player) {
	applyMove(player);
	int ret = isKingInDanger(player);
	applyBack(player);
	return ret;
}

int checkInBetween(int xStep, int yStep, int dif) {
	int x = startX, y = startY, i;
	for (i = 0; i < dif - 1; ++i) {
		x += xStep;
		y += yStep;
		if (grid[x][y].team != 0)
			return 0;
	}
	return 1;
}

int isValidRookMove(int player) {
	if (!(startX == endX && startY != endY)
			&& !(startX != endX && startY == endY))
		return 0;
	int dif, xStep, yStep;
	dif = max(abs(startX - endX), abs(startY - endY));
	xStep = (endX == startX ? 0 : endX > startX ? 1 : -1);
	yStep = (endY == startY ? 0 : endY > startY ? 1 : -1);
	if (!checkInBetween(xStep, yStep, dif))
		return 0;
	return !checkSelfDanger(player);
}

int isValidKnightMove(int player) {
	int xDif, yDif;
	xDif = abs(startX - endX);
	yDif = abs(startY - endY);
	if (!(xDif == 2 && yDif == 1) && !(xDif == 1 && yDif == 2))
		return 0;
	return !checkSelfDanger(player);
}

int isValidBishopMove(int player) {
	if (abs(startX - endX) != abs(startY - endY))
		return 0;
	int dif, xStep, yStep;
	dif = abs(startX - endX);
	xStep = (endX > startX ? 1 : -1);
	yStep = (endY > startY ? 1 : -1);
	if (!checkInBetween(xStep, yStep, dif))
		return 0;
	return !checkSelfDanger(player);
}

int isValidQueenMove(int player) {
	return isValidBishopMove(player) || isValidRookMove(player);
}

int isValidKingMove(int player) {
	int xDif, yDif;
	xDif = abs(startX - endX);
	yDif = abs(startY - endY);
	if (!(xDif == 1 && yDif == 1) && !(xDif == 1 && yDif == 0)
			&& !(xDif == 0 && yDif == 1))
		return 0;
	return !checkSelfDanger(player);
}

int isValidPawnMove(int player) {
	int ok = 0;
	if (player == 1) {
		if (startY == endY && endX - startX == 2 && startX == 1
				&& grid[startX + 1][startY].team == 0
				&& grid[startX + 2][startY].team == 0)
			ok = 1;
		if (startY == endY && endX - startX == 1
				&& grid[startX + 1][startY].team == 0)
			ok = 1;
		if (endX - startX == 1 && abs(startY - endY) == 1
				&& grid[endX][endY].team == 2)
			ok = 1;
	}
	if (player == 2) {
		if (startY == endY && startX - endX == 2 && startX == 6
				&& grid[startX - 1][startY].team == 0
				&& grid[startX - 2][startY].team == 0)
			ok = 1;
		if (startY == endY && startX - endX == 1
				&& grid[startX - 1][startY].team == 0)
			ok = 1;
		if (startX - endX == 1 && abs(startY - endY) == 1
				&& grid[endX][endY].team == 1)
			ok = 1;
	}
	if (!ok)
		return 0;
	return !checkSelfDanger(player);
}

int isValidMove(int player) {
	if (grid[endX][endY].team == player)
		return 0;
	if (grid[startX][startY].team != player)
		return 0;
	char current = grid[startX][startY].symbol;
	current = tolower(current);
	if (current == 'r')
		return isValidRookMove(player);
	if (current == 'n')
		return isValidKnightMove(player);
	if (current == 'b')
		return isValidBishopMove(player);
	if (current == 'k')
		return isValidKingMove(player);
	if (current == 'q')
		return isValidQueenMove(player);
	if (current == 'p')
		return isValidPawnMove(player);
	return 0;
}

int generateMove(int player, int x, int y) {
	startX = x, startY = y;
	int i, j;
	for (i = 1; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			endX = i, endY = j;
			if (isValidMove(player))
				return 1;
		}
	}
	return 0;
}

int canMove(int player) {
	int i, j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			if (grid[i][j].team == player) {
				if (generateMove(player, i, j))
					return 1;
			}
		}
	}
	return 0;
}

char askPrmotion(int player) {
	char input[100];
	printf("Enter new peace (rook - queen - bishop - knight) \n");
	fflush(stdout);
	scanf("%s", input);
	char nullTerminator = 0;
	while (1) {
		char test = toupper(input[0]);
		if (input[1] == nullTerminator)
			if (test == 'R' || test == 'Q' || test == 'B' || test == 'N')
				break;
		printf("Enter valid peace \n");
		fflush(stdout);
		scanf("%s", input);
	}
	if (player == 1)
		input[0] = tolower(input[0]);
	else
		input[0] = toupper(input[0]);
	update[k++] = input[0];
	return input[0];
}

void prmotion() {
	int i;
	for (i = 0; i < 8; ++i) {
		if (grid[7][i].symbol == 'p')
			grid[7][i].symbol = askPrmotion(1);
		if (grid[0][i].symbol == 'P')
			grid[0][i].symbol = askPrmotion(2);
	}
}

void setGame() {
	int i, j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			if ((i + j) & 1)
				grid[i][j].symbol = '*';
			else
				grid[i][j].symbol = '#';
			grid[i][j].team = 0;
		}
	}
	char tmp[] = { 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r' };
	for (i = 0; i < 8; ++i) {
		grid[0][i].symbol = tmp[i];
		grid[1][i].symbol = 'p';
		grid[6][i].symbol = 'P';
		grid[7][i].symbol = toupper(tmp[i]);
		grid[0][i].team = 1;
		grid[1][i].team = 1;
		grid[6][i].team = 2;
		grid[7][i].team = 2;
	}
	ptr[0] = ptr[1] = 0;
	moves = k = 0;
}

void printBoard() {
	int i, j;
	char horizontal[] = { ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };
	for (i = 0; i < 9; ++i) {
		if (i == 0)
			printf(" ");
		printf("%c  ", horizontal[i]);
	}
	printf("\n");
	for (i = 0; i < 8; ++i) {
		if (i == 0)
			printf(" ");
		printf("%d  ", i + 1);
		for (j = 0; j < 8; ++j) {
			printf("%c  ", grid[i][j].symbol);
		}
		printf("%d\n ", i + 1);
	}
	for (i = 0; i < 9; ++i) {
		printf("%c  ", horizontal[i]);
	}
	printf("\n");
	printf("Dead white : \n");
	for (i = 0; i < ptr[0]; ++i) {
		printf("%c ", dead[0][i]);
	}
	printf("\n");
	printf("Dead black : \n");
	for (i = 0; i < ptr[1]; ++i) {
		printf("%c ", dead[1][i]);
	}
	printf("\n");
	fflush(stdout);
}

int isCommand(char input[]) {
	char nullTerminator = 0;
	char current = tolower(input[0]);
	if (current == 's' && input[1] == nullTerminator)
		return 2;
	if (current == 'u' && input[1] == nullTerminator)
		return 3;
	if (current == 'r' && input[1] == nullTerminator)
		return 4;
	return 0;
}

int getInput(int player) {
	char input[100];
	scanf("%s", input);
	if (isValidInput(input) && isValidMove(player))
		return 1;
	if (isCommand(input))
		return isCommand(input);
	printf("Please enter a valid move! \n");
	fflush(stdout);
	return 0;
}

void play() {
	int player;
	while (1) {
		player = (moves & 1 ? 2 : 1);
		printf("Player %d turn : \n", player);
		printf("S to save. \nU to undo. \nR to redo\n");
		fflush(stdout);
		int in = getInput(player);
		while (!in) {
			in = getInput(player);
		}
		if (in == 1) {
			applyMove(player);
			moves++;
			history[0][curr] = startX, history[1][curr] = startY;
			history[2][curr] = endX, history[3][curr] = endY;
			curr++;
			limit = curr;
		} else if (in == 2) {
			save();
			continue;
		} else if (in == 3) {
			undo();
			continue;
		} else if (in == 4) {
			redo();
			continue;
		}
		prmotion();
		printBoard();
		if (isKingInDanger(3 - player)) {
			printf("Check !\n");
			if (!canMove(3 - player)) {
				printf("Checkmate !\n");
				printf("Game Over !\n");
				break;
			}
		} else if (!canMove(3 - player)) {
			printf("Stalemate !\n");
			printf("Game Over !\n");
			break;
		}
	}
}

int main(void) {
	int choice = 0;
	printf("1 - New Game\n2 - Load Game\n");
	fflush(stdout);
	scanf("%d", &choice);
	while (choice != 1 && choice != 2) {
		printf("Enter a valid choice \n");
		fflush(stdout);
		scanf("%d", &choice);
	}
	if (choice == 1) {
		limit = 0;
		setGame();
		printBoard();
		curr = 0;
		play();
	}
	if (choice == 2) {
		limit = 0;
		load();
		play();
	}
	return EXIT_SUCCESS;
}
