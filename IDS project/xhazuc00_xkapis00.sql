--
-- Assignment: IDS project - final part
-- Date: 1/5/2018
-- Authors: Ivan Hazucha (xhazuc00), Marián Kapišinský (xkapis00)
--

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- DROP STRUCTURES
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DROP TABLE osoba CASCADE CONSTRAINTS;
DROP TABLE uzivatel CASCADE CONSTRAINTS;
DROP TABLE recenzia CASCADE CONSTRAINTS;
DROP TABLE kosik CASCADE CONSTRAINTS;
DROP TABLE objednavka CASCADE CONSTRAINTS;
DROP TABLE zamestnanec CASCADE CONSTRAINTS;
DROP TABLE pastelky CASCADE CONSTRAINTS;
DROP TABLE skicak CASCADE CONSTRAINTS;

DROP TABLE kosik_ma_pastelky CASCADE CONSTRAINTS;
DROP TABLE kosik_ma_skicaky CASCADE CONSTRAINTS;

DROP TABLE objednavka_ma_pastelky CASCADE CONSTRAINTS;
DROP TABLE objednavka_ma_skicaky CASCADE CONSTRAINTS;

DROP SEQUENCE ID_OBJEDNAVKA;

DROP MATERIALIZED VIEW MV1;

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- CREATE TABLES
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CREATE TABLE osoba (
	ID         INT GENERATED ALWAYS AS IDENTITY ( START WITH 1 INCREMENT BY 1)     NOT NULL,
	meno       VARCHAR2(256)                                                       NOT NULL,
	priezvisko VARCHAR2(256)                                                       NOT NULL,
	telefon    VARCHAR2(16)                                                        NOT NULL,
	email      VARCHAR2(256) UNIQUE CHECK (email LIKE '%_@__%.__%')                NOT NULL,
	mesto      VARCHAR2(256),
	ulica      VARCHAR2(256),
	cislo_domu VARCHAR2(8),

	PRIMARY KEY (ID)
);


CREATE TABLE uzivatel (
	ID                INT                    NOT NULL,
	login             VARCHAR2(32) UNIQUE    NOT NULL,
	heslo             VARCHAR2(128)          NOT NULL,
	datum_registracie DATE                   NOT NULL,

	FOREIGN KEY (ID) REFERENCES osoba (ID),

	PRIMARY KEY (ID)
);


CREATE TABLE zamestnanec (
	ID              INT                	NOT NULL,
	login           VARCHAR2(32) UNIQUE	NOT NULL,
	heslo           VARCHAR2(128)       NOT NULL,
	zac_prac_pomeru DATE               	NOT NULL,
	kon_prac_pomeru DATE,

	FOREIGN KEY (ID) REFERENCES osoba (ID),

	PRIMARY KEY (ID)
);


CREATE TABLE recenzia (
	ID         INT GENERATED ALWAYS AS IDENTITY ( START WITH 1 INCREMENT BY 1)       NOT NULL,
	hodnotenie SMALLINT CHECK (hodnotenie >= 0 AND hodnotenie <= 5)                  NOT NULL, -- 0 to 5 stars
	sprava     VARCHAR2(2048),
	osoba_id   INT                                                                   NOT NULL, -- foreign key

	FOREIGN KEY (osoba_id) REFERENCES osoba (ID),

	PRIMARY KEY (ID)
);


CREATE TABLE kosik (
	ID              INT GENERATED ALWAYS AS IDENTITY ( START WITH 1 INCREMENT BY 1)      NOT NULL,
	pocet_produktov SMALLINT,
	suma            NUMBER DEFAULT 0,
	uzivatel_id     INT                                                                  NOT NULL,

	FOREIGN KEY (uzivatel_id) REFERENCES uzivatel (ID),

	PRIMARY KEY (ID)
);


CREATE TABLE objednavka (
	ID              INT NOT NULL,
	hmotnost        NUMBER,
	suma            NUMBER DEFAULT 0,
	datum           DATE                                                                 NOT NULL,
	doprava         SMALLINT CHECK (doprava IN (1, 2, 3)), -- 1 'osobne', 2 'posta', 3 'kurier'
	stav            SMALLINT CHECK (stav IN (1, 2, 3, 4)), -- 1 'prijata', 2 'schvalena', 3 'expedovana', 4 'dorucena'
	datum_expedicie DATE,

	zamestnanec_id  INT, -- foreign key
	osoba_id        INT                                                                  NOT NULL, -- foreign key


	FOREIGN KEY (osoba_id) REFERENCES osoba (ID),
	FOREIGN KEY (zamestnanec_id) REFERENCES zamestnanec (ID),

	PRIMARY KEY (ID)

);

CREATE TABLE pastelky (
	ID    INT GENERATED ALWAYS AS IDENTITY ( START WITH 1 INCREMENT BY 1)      NOT NULL,
	farba VARCHAR2(32),
	dlzka NUMBER,
	typ   VARCHAR2(32),
	pocet SMALLINT                                                             NOT NULL,
	cena  NUMBER                                                               NOT NULL,

	PRIMARY KEY (ID)
);

CREATE TABLE skicak (
	ID       INT GENERATED ALWAYS AS IDENTITY ( START WITH 1 INCREMENT BY 1)      NOT NULL,
	format   VARCHAR2(32),
	typ      VARCHAR2(32),
	farba    VARCHAR2(32),
	hmotnost NUMBER,
	pocet    INT                                                                  NOT NULL,
	cena     NUMBER                                                               NOT NULL,

	PRIMARY KEY (ID)
);

-- ====================================
-- Auxiliary tables for N:M relations
-- ====================================

CREATE TABLE kosik_ma_pastelky (
	kosik_id INT NOT NULL,
	pastelky_id INT NOT NULL,

	FOREIGN KEY (kosik_id) REFERENCES kosik(ID),
	FOREIGN KEY (pastelky_id) REFERENCES pastelky(ID),
	CONSTRAINT PK_kosik_pastelky PRIMARY KEY (kosik_id, pastelky_id)
);


CREATE TABLE kosik_ma_skicaky (
	kosik_id INT NOT NULL,
	skicak_id INT NOT NULL,

	FOREIGN KEY (kosik_id) REFERENCES kosik(ID),
	FOREIGN KEY (skicak_id) REFERENCES skicak(ID),
	CONSTRAINT PK_kosik_skicaky PRIMARY KEY (kosik_id, skicak_id)
);


CREATE TABLE objednavka_ma_pastelky (
	objednavka_id INT NOT NULL,
	pastelky_id INT NOT NULL,

	FOREIGN KEY (objednavka_id) REFERENCES objednavka(ID),
	FOREIGN KEY (pastelky_id) REFERENCES pastelky(ID),
	CONSTRAINT PK_objed_pastelky PRIMARY KEY (objednavka_id, pastelky_id)
);


CREATE TABLE objednavka_ma_skicaky (
	objednavka_id INT NOT NULL,
	skicak_id INT NOT NULL,

	FOREIGN KEY (objednavka_id) REFERENCES objednavka(ID),
	FOREIGN KEY (skicak_id) REFERENCES skicak(ID),
	CONSTRAINT PK_objed_skicaky PRIMARY KEY (objednavka_id, skicak_id)
);

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- TRIGGERS
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CREATE SEQUENCE ID_OBJEDNAVKA;

CREATE OR REPLACE TRIGGER OBJEDNAVKA_TRIGGER
BEFORE INSERT ON objednavka
FOR EACH ROW
WHEN (NEW.ID IS NULL)
BEGIN
	SELECT ID_OBJEDNAVKA.nextval INTO :NEW.ID FROM DUAL;
END;

CREATE OR REPLACE TRIGGER REG_TRIGGER
BEFORE INSERT ON uzivatel
FOR EACH ROW
DECLARE CURSOR cursor2 IS SELECT * FROM uzivatel;
BEGIN
	FOR citem IN cursor2
	LOOP
	IF citem.login = :NEW.login THEN
		RAISE_APPLICATION_ERROR(-20002, 'Uzivatel uz existuje!');
	END IF;
	END LOOP;
END;

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- ALTER TABLES
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- ADD DATA
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-- ====================================
-- osoba
-- ====================================

INSERT INTO osoba (meno, priezvisko, telefon, email, mesto, ulica, cislo_domu)
	VALUES ('John', 'Doe', '0918123456', 'johndoe@example.com', 'Ostrava', 'Zelerova', '80');

INSERT INTO osoba (meno, priezvisko, telefon, email, mesto, ulica, cislo_domu)
	VALUES ('Linda', 'Smith', '0918456132', 'lindasmith@example.com', 'Brno', 'Mánesova', '32');

INSERT INTO osoba (meno, priezvisko, telefon, email, mesto, ulica, cislo_domu)
	VALUES ('Jane', 'Doe', '0911123789', 'janedoe@example.com', 'Ostrava', 'Zelerova', '80');


-- ====================================
-- uzivatel
-- ====================================

-- Reálne by sa v systéme mohla pri registrácií uživateľa prehľadávať tabuľka osôb napríklad
-- na základe emailu, prípadne kombinácie mena, priezviska a bydliska. Ak by sa podarilo
-- príslušný záznam nájsť, ID tohoto záznamu bude pridané k registrovanému uživateľovi
-- spolu so zadanými registračnými údajmi. V opačnom prípade by bol najskôr vytvorený záznam
-- v tabuľke osoba a následne vytvorený uživateľ s daným ID

INSERT INTO uzivatel(login, heslo, datum_registracie, ID)
	VALUES ('jodoe', 'password123', (SELECT SYSDATE FROM DUAL), (SELECT ID FROM osoba WHERE email = 'johndoe@example.com'));

INSERT INTO uzivatel(login, heslo, datum_registracie, ID)
	VALUES ('lismith', 'password465', (SELECT SYSDATE FROM DUAL), (SELECT ID FROM osoba WHERE email = 'lindasmith@example.com'));

-- ====================================
-- zamestnanec
-- ====================================

INSERT INTO zamestnanec(login, heslo, zac_prac_pomeru, ID)
	VALUES ('janedoe', 'securepswd123', (SELECT SYSDATE FROM DUAL), (SELECT ID FROM osoba WHERE email = 'janedoe@example.com'));

-- ====================================
-- recenzia
-- ====================================

INSERT INTO recenzia(hodnotenie, sprava, osoba_id)
	VALUES (3, 'Táto správa je hodnotenie obchodu', (SELECT ID FROM osoba WHERE email = 'lindasmith@example.com'));

INSERT INTO recenzia(hodnotenie, sprava, osoba_id)
	VALUES (5, 'Výborný výber produktov', (SELECT ID FROM osoba WHERE email = 'janedoe@example.com'));

-- ====================================
-- kosik
-- ====================================

INSERT INTO kosik(pocet_produktov, suma, uzivatel_id)
	VALUES (1, 200, (SELECT ID FROM osoba WHERE email = 'lindasmith@example.com'));

INSERT INTO kosik(pocet_produktov, suma, uzivatel_id)
	VALUES (2, 180, (SELECT ID FROM osoba WHERE email = 'johndoe@example.com'));

INSERT INTO kosik(pocet_produktov, suma, uzivatel_id)
	VALUES (2, 300, (SELECT ID FROM osoba WHERE email = 'johndoe@example.com'));

-- ====================================
-- objednavka
-- ====================================

INSERT INTO objednavka(ID, hmotnost, suma, datum, doprava, stav, osoba_id, zamestnanec_id)
	VALUES (ID_OBJEDNAVKA.nextval, 530, 450, (SELECT SYSDATE FROM DUAL), 1, 2, (SELECT ID FROM osoba WHERE email = 'lindasmith@example.com'),
		(SELECT ID FROM osoba WHERE email = 'janedoe@example.com')
	);

INSERT INTO objednavka(ID, hmotnost, suma, datum, datum_expedicie, doprava, stav, osoba_id, zamestnanec_id)
	VALUES (ID_OBJEDNAVKA.nextval, 500, 200, (TO_DATE('2018-03-30','YYYY-MM-DD')), (TO_DATE('2018-03-31','YYYY-MM-DD')), 2, 3,
		(SELECT ID FROM osoba WHERE email = 'johndoe@example.com'), (SELECT ID FROM osoba WHERE email = 'janedoe@example.com')
	);

-- ====================================
-- pastelky
-- ====================================

INSERT INTO pastelky(farba, dlzka, typ, pocet, cena)
	VALUES ('rozne', 175, 'grafit', 10, 200);

INSERT INTO pastelky(farba, dlzka, typ, pocet, cena)
	VALUES ('rozne', 90, 'vosk', 10, 80);

INSERT INTO pastelky(farba, dlzka, typ, pocet, cena)
	VALUES ('cervene', 150, 'grafit', 15, 250);

INSERT INTO pastelky(farba, dlzka, typ, pocet, cena)
	VALUES ('zelene', 150, 'grafit', 15, 250);

-- ====================================
-- skicak
-- ====================================

INSERT INTO skicak(format, typ, farba, hmotnost, pocet, cena)
	VALUES ('A4', 'kancelarsky', 'biela', 500, 100, 100);

INSERT INTO skicak(format, typ, farba, hmotnost, pocet, cena)
	VALUES ('A4', 'kancelarsky', 'biela', 250, 50, 50);

INSERT INTO skicak(format, typ, farba, hmotnost, pocet, cena)
	VALUES ('A3', 'kancelarsky', 'rozne', 500, 50, 200);

INSERT INTO skicak(format, typ, farba, hmotnost, pocet, cena)
	VALUES ('A4', 'hruby', 'cervena', 500, 75, 200);

-- ====================================
-- kosik_ma_pastelky
-- ====================================

INSERT INTO kosik_ma_pastelky(kosik_id, pastelky_id)
	VALUES (1, 1);

INSERT INTO kosik_ma_pastelky(kosik_id, pastelky_id)
	VALUES (2, 2);

INSERT INTO kosik_ma_pastelky(kosik_id, pastelky_id)
	VALUES (3, 3);

-- ====================================
-- kosik_ma_skicaky
-- ====================================

INSERT INTO kosik_ma_skicaky(kosik_id, skicak_id)
	VALUES (2, 1);

INSERT INTO kosik_ma_skicaky(kosik_id, skicak_id)
	VALUES (3, 2);

-- ====================================
-- objednavka_ma_pastelky
-- ====================================

INSERT INTO objednavka_ma_pastelky(objednavka_id, pastelky_id)
	VALUES (1,4);

-- ====================================
-- objednavka_ma_skicaky
-- ====================================

INSERT INTO objednavka_ma_skicaky(objednavka_id, skicak_id)
	VALUES (1,3);

INSERT INTO objednavka_ma_skicaky(objednavka_id, skicak_id)
	VALUES (2,4);

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- SELECT
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-- ====================================
-- Select mena a priezviska ludi, ktorych objednavka je nad 400 korun
-- ====================================

SELECT DISTINCT meno, priezvisko
  FROM osoba JOIN objednavka o ON osoba.ID = o.osoba_id
  WHERE suma > 400;

-- ====================================
-- Select sprav recenzii od uzivatela Jane Doe
-- ====================================

SELECT sprava
  FROM osoba JOIN recenzia r ON osoba.ID = r.osoba_id
  WHERE meno = 'Jane' AND priezvisko = 'Doe';

-- ====================================
-- Select mena a priezviska cloveka s najviac polozkami v kosiku
-- ====================================

SELECT DISTINCT meno, priezvisko
  FROM osoba JOIN uzivatel u ON osoba.ID = u.ID JOIN kosik k ON u.ID = k.uzivatel_id
  WHERE (pocet_produktov) IN
  ( SELECT MAX(pocet_produktov)
      FROM kosik
  );

-- ====================================
-- Select pre vypisanie poctu objednavok do vsetkych miest v abecednom poradi
-- ====================================

SELECT osoba.mesto, COUNT(objednavka.ID)
  FROM objednavka JOIN osoba ON objednavka.osoba_id = osoba.ID
  GROUP BY mesto
  ORDER BY mesto ASC;

-- ====================================
-- Select uživateľa, ktorý má v košíku pastelky
-- ====================================

SELECT DISTINCT meno, priezvisko
	FROM osoba
	WHERE EXISTS(
		SELECT kosik.ID
			FROM kosik JOIN kosik_ma_pastelky kmp ON kosik.ID = kmp.kosik_id
			WHERE kosik.uzivatel_id = osoba.ID
	);

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- PROCEDURES
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CREATE OR REPLACE PROCEDURE POCET_PRODUKTOV
AS
	POCET_S NUMBER;
	POCET_P NUMBER;
BEGIN
	SELECT COUNT(*) INTO POCET_S
		FROM objednavka_ma_skicaky oms JOIN objednavka o ON oms.objednavka_id = o.ID
		WHERE	o.datum BETWEEN TRUNC(SYSDATE) AND TRUNC(SYSDATE +7);
	SELECT COUNT (*) INTO POCET_P
		FROM objednavka_ma_pastelky omp JOIN objednavka o ON omp.objednavka_id = o.ID
		WHERE	o.datum BETWEEN TRUNC(SYSDATE) AND TRUNC(SYSDATE +7);
	DBMS_OUTPUT.put_line('Pocet objednaných skicákov: ' || POCET_S);
	DBMS_OUTPUT.put_line('Pocet objednaných pasteliek: ' || POCET_P);
END;

CREATE OR REPLACE PROCEDURE EXP_OBJEDNAVOK
AS
	doprava VARCHAR2(16);
	id_tmp objednavka.ID%type;
	doprava_tmp objednavka.doprava%type;
	CURSOR cursor1 IS SELECT * FROM objednavka WHERE objednavka.stav = 2;
BEGIN
	FOR item IN cursor1
	LOOP
    id_tmp := item.ID;
    doprava_tmp := ITEM.doprava;
    IF doprava_tmp = 1 THEN
      doprava := 'Osobne';
    END IF;
    IF doprava_tmp = 2 THEN
      doprava := 'Posta';
    END IF;
    IF doprava_tmp = 3 THEN
      doprava := 'Kurier';
    END IF;
		DBMS_OUTPUT.put_line('Doprava objednávky ' || id_tmp || ' je ' || doprava);
	END LOOP;
END;

CALL POCET_PRODUKTOV();

CALL EXP_OBJEDNAVOK();

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- EXPLAIN PLAN
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EXPLAIN PLAN FOR
SELECT meno, priezvisko, COUNT(o2.id) AS POCET_VYBAVENYCH_OBJEDNAVOK
FROM osoba o JOIN zamestnanec z ON o.ID = z.ID JOIN objednavka o2 ON z.ID = o2.zamestnanec_id
WHERE o2.datum BETWEEN TRUNC(SYSDATE) AND TRUNC(SYSDATE +7)
GROUP BY meno, priezvisko;

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY);

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- EXPLAIN PLAN WITH INDEX
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CREATE INDEX objednavka_index ON objednavka(datum);

EXPLAIN PLAN FOR
SELECT meno, priezvisko, COUNT(o2.id) AS POCET_VYBAVENYCH_OBJEDNAVOK
FROM osoba o JOIN zamestnanec z ON o.ID = z.ID JOIN objednavka o2 ON z.ID = o2.zamestnanec_id
WHERE o2.datum BETWEEN TRUNC(SYSDATE) AND TRUNC(SYSDATE +7)
GROUP BY meno, priezvisko;

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY);

DROP INDEX objednavka_index;

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- RIGHTS
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GRANT ALL ON osoba TO XHAZUC00;
GRANT ALL ON uzivatel TO XHAZUC00;
GRANT ALL ON recenzia TO XHAZUC00;
GRANT ALL ON kosik TO XHAZUC00;
GRANT ALL ON objednavka TO XHAZUC00;
GRANT ALL ON zamestnanec TO XHAZUC00;
GRANT ALL ON pastelky TO XHAZUC00;
GRANT ALL ON skicak TO XHAZUC00;
GRANT ALL ON kosik_ma_pastelky TO XHAZUC00;
GRANT ALL ON kosik_ma_skicaky TO XHAZUC00;
GRANT ALL ON objednavka_ma_pastelky TO XHAZUC00;
GRANT ALL ON objednavka_ma_skicaky TO XHAZUC00;

GRANT EXECUTE ON POCET_PRODUKTOV TO XHAZUC00;
GRANT EXECUTE ON EXP_OBJEDNAVOK TO XHAZUC00;

-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-- MATERIALIZED VIEW
-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CREATE MATERIALIZED VIEW MV1
NOLOGGING
CACHE
BUILD IMMEDIATE
REFRESH ON COMMIT
AS
  SELECT hmotnost, suma, datum, doprava, stav, datum_expedicie, meno, priezvisko  FROM objednavka o JOIN osoba o2 ON o.osoba_id = o2.ID;

GRANT ALL ON MV1 TO XHAZUC00;
