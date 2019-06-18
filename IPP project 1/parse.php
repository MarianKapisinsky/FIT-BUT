<?php

define("ARG_ERROR", 10);
define("PARSE_ERROR",21);

$lines_of_code = 0;
$lines_of_comments = 0;

$instructions = array
	(
		array("MOVE", 2),
		array("CREATEFRAME", 0),
		array("PUSHFRAME", 0),
		array("POPFRAME", 0),
		array("DEFVAR", 1),
		array("CALL", 1),
		array("RETURN", 0),
		array("PUSHS", 1),
		array("POPS", 1),
		array("ADD", 3),
		array("SUB", 3),
		array("MUL", 3),
		array("IDIV", 3),
		array("LT", 3),
		array("GT", 3),
		array("EQ", 3),
		array("AND", 3),
		array("OR", 3),
		array("NOT", 2),
		array("INT2CHAR", 2),
		array("STRI2INT", 3),
		array("READ", 2),
		array("WRITE", 1),
		array("CONCAT", 3),
		array("STRLEN", 2),
		array("GETCHAR", 3),
		array("SETCHAR", 3),
		array("TYPE", 2),
		array("LABEL", 1),
		array("JUMP", 1),
		array("JUMPIFEQ", 3),
		array("JUMPIFNEQ", 3),
		array("DPRINT", 1),
		array("BREAK", 0)
	);
	
$patterns = array('/^int@[+-]?[[:digit:]]+$/', '/^bool@(true|false)$/', '/^string@(([^#\s\a\e\f\n\r\t\v\\\]*)|\\\[[:digit:]]{3})*$/', '/^[LTG]F@([\Q-*\E_$&%]*|[[:alnum:]]+)+$/');

function parseLine($line) {
	
	global $lines_of_comments;
	
	if (strpos($line, '#')) {
		
		$line = preg_replace('~(#[^\a\e\f\n\r\v]*)([^\a\e\f\n\r\v]+)*~',"",$line);
		$lines_of_comments++;
	}
	
	$line = trim($line);
	
    $keywords = preg_split('/[\s\t]+/', $line);
	
	return $keywords;
}

function checkKeyword($keywords, $instructions) {
	
	$found = false;
	
	for ($i = 0; $i < 34; $i++) {

		if (strcmp($keywords[0], $instructions[$i][0]) == 0) {
			
			$found = true;
			break;
		}
		else {
			
			$found = false;
		}	
	}
	if ($found == false) { exit(PARSE_ERROR); }
	
}

function checkArgCount($keywords, $instructions) {
	
	for ($i = 0; $i < 34; $i++) {

		if (strcmp($keywords[0], $instructions[$i][0]) == 0) {
			
			if ((count($keywords) - 1) == $instructions[$i][1]) {
				
				break;
			}
			else {
				exit(PARSE_ERROR);
			}
		}	
	}
}

function checkVar($arg) {

	if (!preg_match('/^[LTG]F@([\Q-*\E_$&%]*|[[:alnum:]]+)+$/', $arg)) {

		exit(PARSE_ERROR);
	}
}

function checkSymb($arg, $patterns) {
	
	$match = false;

	foreach ($patterns as $pattern) {

		if (preg_match($pattern, $arg)) {

			$match= true;
			break;
		}
		else { 
		
			$match = false;
		}
	}
	if ($match == false) { exit(PARSE_ERROR); }

}

function checkLabel($arg) {

	if (!preg_match('/^([\Q-*\E_$&%]*|[[:alnum:]]+)+$/', $arg)) {
	
		exit(PARSE_ERROR);
	}
}

function checkArgs($keywords, $patterns) {

	switch ($keywords[0]) {
		
		case "MOVE"		:
		case "NOT"		:
		case "INT2CHAR" :
		case "STRLEN"	:
		case "TYPE"     :
			
			checkVar($keywords[1]);
			checkSymb($keywords[2], $patterns);
			break;
			
		case "DEFVAR" :
		case "POPS"   :
			
			checkVar($keywords[1]);
			break;
			
		case "CALL"  :
		case "LABEL" :
		case "JUMP"  :
			
			checkLabel($keywords[1]);
			break;
		
		case "PUSHS"  :
		case "WRITE"  :
		case "DPRINT" :
		
			checkSymb($keywords[1], $patterns);
			break;
			
		case "ADD"		:
		case "SUB"		:
		case "MUL"		:
		case "IDIV" 	:
		case "LT"		:
		case "GT"		:
		case "EQ"		:
		case "AND"		:
		case "OR"		:
		case "STRI2INT" :
		case "CONCAT"	:
		case "GETCHAR"	:
		case "SETCHAR"	:
			
			checkVar($keywords[1]);
			checkSymb($keywords[2], $patterns);
			checkSymb($keywords[3], $patterns);
			break;
		
		case "READ" :
			
			checkVar($keywords[1]);
						
			if (!preg_match('/^(int|bool|string)$/', $keywords[2])) {
	
				exit(PARSE_ERROR);
			}
			break;
		
		case "JUMPIFEQ"  :
		case "JUMPIFNEQ" :
			
			checkLabel($keywords[1]);
			checkSymb($keywords[2], $patterns);
			checkSymb($keywords[3], $patterns);
			break;
			
		default :
			
			break;
	}
}

function writeXML($keywords, $order, $xml) {

	$instruction = $xml->addChild('instruction');
	$instruction->addAttribute('order', "$order");
	$instruction->addAttribute('opcode', "$keywords[0]");

	for ($i = 1; $i < sizeof($keywords); ++$i) {
		
		if (preg_match('/@/', $keywords[$i])) {
				
			$arg = explode("@", $keywords[$i]);
			
			$arg[1] = str_replace("&", "&amp;", $arg[1]);
			$arg[1] = str_replace("<", "&lt;", $arg[1]);
			$arg[1] = str_replace(">", "&gt;", $arg[1]);
			$arg[1] = str_replace('"', "&quot;", $arg[1]);
			
			if (preg_match('/[LTG]F/', $arg[0])) { 
			
				$arg[1] = $arg[0] . "@" . $arg[1];
				$arg[0] = "var"; 
			}
			$arg_xml = $instruction->addChild("arg$i", $arg[1]);
			$arg_xml->addAttribute('type', "$arg[0]");
		}
		else {
			
			$keywords[1] = str_replace("&", "&amp;", $keywords[1]);
			$keywords[1] = str_replace("<", "&lt;", $keywords[1]);
			$keywords[1] = str_replace(">", "&gt;", $keywords[1]);
			$keywords[1] = str_replace('"', "&quot;", $keywords[1]);
				
			$arg_xml = $instruction->addChild("arg$i", $keywords[$i]);
			$arg_xml->addAttribute('type', 'label');
		}
		
	}
}

$statp = false;
$loc = false;
$comments = false;

array_shift($argv);

if (isset($argv)) {
	
	foreach ($argv as $cmd_arg) {
				
		if (strcmp($cmd_arg, '--help') == 0 ) {
				
			echo "Skript typu filtr (parse.php v jazyce PHP 5.6) nacte ze standardniho vstupu zdrojovy kod v IPPcode18,\nzkontroluje lexikalni a syntaktickou spravnost kodu a vypise na standardni vystup XML reprezentaci programu\nParametry:\n--help - zobrazi napovedu\n";
			exit(0);
		}
		elseif (preg_match('/--stats=.+/', $cmd_arg)) {

			$filename = substr($cmd_arg, strpos($cmd_arg, "=") + 1); 
			$statp = true;
		}
		elseif (strcmp($cmd_arg, '--loc') == 0 ) {
			
			if ($statp == false) { exit(ARG_ERROR); }
			$loc = true;
		}
		elseif (strcmp($cmd_arg, '--comments') == 0 ) {
			
			if ($statp == false) { exit(ARG_ERROR); }
			$comments = true;
		}
		else {
		
			exit(ARG_ERROR);
		}
	}
}

if ($statp) {

	if (($loc == false) && ($comments == false)) {
	
		exit(ARG_ERROR);
	}
}

$line = fgets(STDIN);
$line = trim($line);

if (strpos($line, '#')) {
		
	$line = preg_replace('~(#[^\a\e\f\n\r\v]*)([^\a\e\f\n\r\v]+)*~',"",$line);
	$lines_of_comments++;
}

if (strcasecmp($line, ".IPPcode18") != 0) {
	
	exit(PARSE_ERROR);
}

$xml_out = <<<XML
<?xml version='1.0' encoding='UTF-8'?>
<program language='IPPcode18'>
</program>
XML;

$order = 1;
$xml = new SimpleXMLElement($xml_out);

while ($line = fgets(STDIN)) {

	$line = trim($line, " \t\r\0\x0B");
	if (strcmp($line[0],"#") == 0) { 
	
		$lines_of_comments++;
		continue; 
	}
	if (strcmp($line[0],"\n") == 0) { continue; }
	$keywords = parseLine($line, $lines_of_comments);
	$keywords[0] = strtoupper($keywords[0]);
	
	checkKeyword($keywords, $instructions);
	checkArgCount($keywords, $instructions);
	checkArgs($keywords, $patterns);
	
	writeXML($keywords, $order, $xml);
	$order++;
	$lines_of_code++;
}

$dom = new DOMDocument("1.0");
$dom->preserveWhiteSpace = false;
$dom->formatOutput = true;
$dom->loadXML($xml->asXML());
echo $dom->saveXML();

if ($statp) {
	
	$file = fopen($filename, "w") or die("Unable to open file!");
	
	if ($loc) { fwrite($file, $lines_of_code . "\n"); }
	if ($comments) { fwrite($file, $lines_of_comments . "\n"); }

	fclose($file);
}

?>
