//The scripts for the tooltip and moveobj are based on work from
//SVG - Learning By Coding - http://www.datenverdrahten.de/svglbc/
//Author: Dr. Thomas Meinike 11/03 - thomas@handmadecode.de
var svgdoc,svgroot,paths,revert;
var collapsed = new Array(2);
var name = new Array(%i);
var clickStatus = new Array(%i);
var redCount = new Array(%i);
var salmonCount = new Array(%i);
var moveObj,rect,x,y,attr1,attr2,wert1,wert2,zoom=1,active=false;

function MoveObj(evt)
{
    if (active) {
	var roundx, roundy;
	var curtrans=svgroot.currentTranslate;
	var ctx=curtrans.x;
	var cty=curtrans.y;

	x=evt.clientX()
	y=evt.clientY()
	wert1=(x-ctx)*zoom*%.2f+%.2f+attr1-5;
	wert2=(y-cty)*zoom*%.2f+%.2f+attr2-5;
	roundx=Math.round(wert1*100)/100;
	roundy=Math.round(wert2*100)/100;

	moveObj.setAttribute("transform","translate("+roundx+","+roundy+")");
    }
}

function ClickObj(evt)
{
    rect=evt.target.parentNode.getElementsByTagName("rect").item(0);
    moveObj=evt.target.parentNode;
    attr1=rect.getAttribute("width")/2;
    attr2=rect.getAttribute("height")/2;
    active=true;
}

function OutOfObj(evt)
{
    if (active) {
	active=false;
	findAdjacentEdges();
    }
}

function findAdjacentEdges()
{
    var rectl, rectlid, i, nodenames;
    rectl = rect;
    rectlid = rectl.getAttribute("id");
    for (i=0; i<paths.length; i++) {
	nodenames = paths.item(i).getAttribute("id").split("-");
	if (nodenames[0] == rectlid || nodenames[1] == rectlid) {
	    repaintEdge(paths.item(i), nodenames, i);
	}
    }
}

function repaintEdge(edge, nodenames, j)
{
    var startnode, endnode, attr, i, k, l, m, alpha, beta;
    var nodesx, nodesy, nodeex, nodeey, nodesw, nodesh, nodeew, nodeeh;
    var edgesx, edgesy, edgeex, edgeey, sx, sy, ex, ey;

    //extract node coordinates and dimensions
    startnode = svgdoc.getElementById(nodenames[0]);
    nodesw=startnode.getAttribute("width");
    nodesh=startnode.getAttribute("height");
    attr = startnode.parentNode.attributes;
    for (i=0;i<attr.length;i++) {
        if (attr.item(i).nodeName == "transform") {
	    k = attr.item(i).nodeValue.indexOf("(");
	    l = attr.item(i).nodeValue.indexOf(",");
	    m = attr.item(i).nodeValue.indexOf(")");
	    nodesx = parseFloat(attr.item(i).nodeValue.substring(k+1,l));
	    nodesy = parseFloat(attr.item(i).nodeValue.substring(l+1,m));
	}
    }
    endnode = svgdoc.getElementById(nodenames[1]);
    nodeew=endnode.getAttribute("width");
    nodeeh=endnode.getAttribute("height");
    attr = endnode.parentNode.attributes;
    for (i=0;i<attr.length;i++) {
        if (attr.item(i).nodeName == "transform") {
	    k = attr.item(i).nodeValue.indexOf("(");
	    l = attr.item(i).nodeValue.indexOf(",");
	    m = attr.item(i).nodeValue.indexOf(")");
	    nodeex = parseFloat(attr.item(i).nodeValue.substring(k+1,l));
	    nodeey = parseFloat(attr.item(i).nodeValue.substring(l+1,m));
	}
    }

    alpha = Math.atan((nodesy-nodeey)/(nodesx-nodeex));
    if (alpha < 0)
	alpha += Math.PI;

    //calculate intersection of edge and startNode
    beta = Math.atan(nodesh/nodesw);
    if (alpha < beta
	|| (alpha > Math.PI-beta && alpha < Math.PI+beta)
	|| alpha > 2*Math.PI-beta) {
	//intersection at left or right border
	if (nodesx < nodeex) {
	    edgesx = nodesx - 0 + nodesw/2;
	} else {
	    edgesx = nodesx - nodesw/2;
	}
	if (nodesy < nodeey) {
	    edgesy = nodesy - 0 + Math.abs(nodesw*Math.tan(alpha)/2);
	} else {
	    edgesy = nodesy - Math.abs(nodesw*Math.tan(alpha)/2);
	}
    } else {
	//intersection at top or bottom border
	if (nodesy < nodeey) {
	    edgesy = nodesy - 0 + nodesh/2;
	} else {
	    edgesy = nodesy - nodesh/2;
	}
	if (nodesx < nodeex) {
	    edgesx = nodesx - 0 + Math.abs(nodesh/(2*Math.tan(alpha)));
	} else {
	    edgesx = nodesx - Math.abs(nodesh/(2*Math.tan(alpha)));
	}
    }

    //calculate intersection of edge and endNode
    beta = Math.atan(nodeeh/nodeew);
    if (alpha < beta
	|| (alpha > Math.PI-beta && alpha < Math.PI+beta)
	|| alpha > 2*Math.PI-beta) {
	//intersection at left or right border
	if (nodesx > nodeex) {
	    edgeex = nodeex - 0 + nodeew/2;
	} else {
	    edgeex = nodeex - nodeew/2;
	}
	if (nodesy > nodeey) {
	    edgeey = nodeey - 0 + Math.abs(nodeew*Math.tan(alpha)/2);
	} else {
	    edgeey = nodeey - Math.abs(nodeew*Math.tan(alpha)/2);
	}
    } else {
	//intersection at top or bottom border
	if (nodesy > nodeey) {
	    edgeey = nodeey - 0 + nodeeh/2;
	} else {
	    edgeey = nodeey - nodeeh/2;
	}
	if (nodesx > nodeex) {
	    edgeex = nodeex - 0 + Math.abs(nodeeh/(2*Math.tan(alpha)));
	} else {
	    edgeex = nodeex - Math.abs(nodeeh/(2*Math.tan(alpha)));
	}
    }

    //set new edge coordinates
    sx=Math.round(edgesx*100)/100;
    ex=Math.round(edgeex*100)/100;
    sy=Math.round(edgesy*100)/100;
    ey=Math.round(edgeey*100)/100;

    if (sx < ex) {
	if (revert[j] == 1) {
	    revert[j] = 0;
	    flipEdgeMarks(edge);
	}
	edge.setAttribute("d","M "+sx+" "+sy+" "+ex+" "+ey);
    } else {
	if (revert[j] == 0) {
	    revert[j] = 1;
	    flipEdgeMarks(edge);
	}
	edge.setAttribute("d","M "+ex+" "+ey+" "+sx+" "+sy);
    }
}

function flipEdgeMarks(edge)
{
    var attr, i, j, done, textpaths, pathnamei, offset;

    //revert start- and end-markers
    attr = edge.attributes;
    done = 0;
    for (i=0;i<attr.length;i++) {
        if (attr.item(i).nodeName == "marker-start") {
	    for (j=i;j<attr.length;j++) {
		if (attr.item(j).nodeName == "marker-end") {
		    done = 1;
		}
	    }
	    if (done == 0) {
		//start->end
		edge.removeAttribute("marker-start");
		edge.setAttribute("marker-end","url(#arrowend)");
		break;
	    }
	}
        if (attr.item(i).nodeName == "marker-end") {
	    for (j=i;j<attr.length;j++) {
		if (attr.item(j).nodeName == "marker-start") {
		    done = 1;
		}
	    }
	    if (done == 0) {
		//end->start
		edge.removeAttribute("marker-end");
		edge.setAttribute("marker-start","url(#arrowstart)");
		break;
	    }
	}
    }

    //revert cardinalities
    textpaths = svgdoc.getElementsByTagName("textPath");
    for (i=0;i<textpaths.length;i++) {
	pathname = textpaths.item(i).getAttribute("xlink:href").replace(/#/,"");
	if (pathname == edge.getAttribute("id")) {
	    offset = textpaths.item(i).getAttribute("startOffset").substr(0,2);
	    textpaths.item(i).setAttribute("startOffset",100 - offset + "%")
	}
    }
}

function getSVGDoc(load_evt)
{
    svgdoc=load_evt.target.ownerDocument;
    svgroot=svgdoc.documentElement;
    texte=svgdoc.getElementById("tooltip").getElementsByTagName("text");
    for (i=1; i<%i; i++) {
	textNode=texte.item(1).cloneNode("true");
	svgdoc.getElementById("tooltip").appendChild(textNode);
    }
    texte=svgdoc.getElementById("tooltip").getElementsByTagName("text");
}

function ShowTooltipMZ(mousemove_event,txt)
{
    var ttrelem,tttelem,ttline,posx,posy,curtrans,ctx,cty,txt;
    var maxbreite,tmp,i;
    ttrelem=svgdoc.getElementById("ttr");
    tttelem=svgdoc.getElementById("ttt");
    ttline=svgdoc.getElementById("ttl");
    posx=mousemove_event.clientX;
    posy=mousemove_event.clientY;
    for (i=1;i<=%i;i++)
	texte.item(i).firstChild.data="";
    tttelem.childNodes.item(0).data="";
    tmp=txt.split("\n");
    maxbreite=0;
    for (i=0;i<tmp.length;i++) {
	if (tmp[i]=="")
	    continue;
	tttelem.childNodes.item(0).data=tmp[i];
	if (maxbreite<tttelem.getComputedTextLength())
	    maxbreite=tttelem.getComputedTextLength();
    }
    curtrans=svgroot.currentTranslate;
    ctx=curtrans.x;
    cty=curtrans.y;
    ttrelem.setAttribute("x",posx-ctx+10);
    ttrelem.setAttribute("y",posy-cty-20+10);
    ttrelem.setAttribute("width",maxbreite*0.92+10);
    ttrelem.setAttribute("height",tmp.length*15+3);
    ttrelem.setAttribute("style",
			    "fill: #FFC; stroke: #000; stroke-width: 0.5px");
    for (i=1; i<=tmp.length; i++) {
	if (tmp[i-1]=="-- -- --") {
	    ttline.setAttribute("x1", posx-ctx+10);
	    ttline.setAttribute("y1", parseInt(i-1)*15+posy-cty);
	    ttline.setAttribute("x2", posx-ctx+10+maxbreite*0.92+10);
	    ttline.setAttribute("y2", parseInt(i-1)*15+posy-cty);
	    ttline.setAttribute("style", "stroke: #000; stroke-width: 0.5px");
	} else {
	    texte.item(i).firstChild.data=tmp[i-1];
	    texte.item(i).setAttribute("x",posx-ctx+15);
	    texte.item(i).setAttribute("y",parseInt(i-1)*15+posy-cty+3);
	    texte.item(i).setAttribute("style","fill: #00E; font-size: 11px");
	}
    }
    svgdoc.getElementById("tooltip").style.setProperty("visibility","visible");
}

function HideTooltip()
{
    svgdoc.getElementById("ttl").style.setProperty("visibility","hidden");
    svgdoc.getElementById("tooltip").style.setProperty("visibility","hidden");
}

function ZoomControl()
{
    var curzoom;
    curzoom=svgroot.currentScale;
    svgdoc.getElementById("tooltip").setAttribute("transform",
							"scale("+1/curzoom+")");
    zoom=1/curzoom;
}

function collapse(evt)
{
    var i, k, l, m, svgdoc, obj, targetID, targetX, targetY, attr;
    obj = evt.getTarget();
    svgdoc = obj.ownerDocument;
    //extract coordinates and id of the clicked text
    attr = obj.parentNode.parentNode.attributes;
    for (i=0;i<attr.length;i++) {
        if (attr.item(i).nodeName == "transform") {
            k = attr.item(i).nodeValue.indexOf("(");
            l = attr.item(i).nodeValue.indexOf(",");
            m = attr.item(i).nodeValue.indexOf(")");
            targetX = attr.item(i).nodeValue.substring(k+1,l);
            targetY = attr.item(i).nodeValue.substring(l+1,m);
        }
        if (attr.item(i).nodeName == "id") {
            targetID = attr.item(i).nodeValue.substr(2);
        }
    }
    //decide if we are collapsing or uncollapsing
    if (collapsed[0][targetID] == 0) {
        hideInfos(evt, obj, svgdoc, targetX, targetY, targetID, attr);
        collapsed[0][targetID] = 1;
    } else {
        showHiddenInfos(evt, obj, svgdoc, targetX, targetY, targetID, attr);
        collapsed[0][targetID] = 0;
    }
}

function showHiddenInfos(evt, obj, svgdoc, targetX, targetY, targetID, attr)
{
    var i, k, l, m, nextObj, nextX, nextY, gapY=0, clickedID;
    //change clicked text
    obj.firstChild.data="--";
    targetID++;
    nextObj = svgdoc.getElementById("MI"+targetID);
    if (nextObj == null)
        return;
    //show child texts again
    for (;;) {
        attr = nextObj.attributes;
        for (i=0;i<attr.length;i++) {
            if (attr.item(i).nodeName == "transform") {
                k = attr.item(i).nodeValue.indexOf("(");
                l = attr.item(i).nodeValue.indexOf(",");
                m = attr.item(i).nodeValue.indexOf(")");
                nextX = attr.item(i).nodeValue.substring(k+1,l);
                nextY = attr.item(i).nodeValue.substring(l+1,m);
            }
        }
        if (nextX > targetX) {
            nextObj.style.setProperty("visibility","visible");
            gapY += 15;
            if (collapsed[0][targetID] == 1) {
                targetID = collapsed[1][targetID];
            } else {
                targetID++;
            }
            nextObj = svgdoc.getElementById("MI"+targetID);
            if (nextObj == null)
                break;
        } else {
            break;
        }
    }
    if (nextObj == null)
        return;
    //move following texts downwards
    while (nextObj != null) {
        attr = nextObj.attributes;
        for (i=0;i<attr.length;i++) {
            if (attr.item(i).nodeName == "transform") {
                k = attr.item(i).nodeValue.indexOf("(");
                l = attr.item(i).nodeValue.indexOf(",");
                m = attr.item(i).nodeValue.indexOf(")");
                nextX = attr.item(i).nodeValue.substring(k+1,l);
                nextY = attr.item(i).nodeValue.substring(l+1,m);
            }
        }
        nextY = nextY - 1 + gapY + 1;
        nextObj.setAttribute("transform","translate("+nextX+","+nextY+")");
        targetID++;
        nextObj = svgdoc.getElementById("MI"+targetID);
    }
}

function hideInfos(evt, obj, svgdoc, targetX, targetY, targetID, attr)
{
    var i, k, l, m, nextObj, nextX, nextY, gapY=0;
    clickedID = targetID;
    //change clicked text
    obj.firstChild.data="+";
    targetID++;
    nextObj = svgdoc.getElementById("MI"+targetID);
    if (nextObj == null)
        return;
    //wipe out child texts
    for (;;) {
        attr = nextObj.attributes;
        for (i=0;i<attr.length;i++) {
            if (attr.item(i).nodeName == "transform") {
                k = attr.item(i).nodeValue.indexOf("(");
                l = attr.item(i).nodeValue.indexOf(",");
                m = attr.item(i).nodeValue.indexOf(")");
                nextX = attr.item(i).nodeValue.substring(k+1,l);
                nextY = attr.item(i).nodeValue.substring(l+1,m);
            }
        }
        if (nextX > targetX) {
            nextObj.style.setProperty("visibility","hidden");
            gapY += 15;
            if (collapsed[0][targetID] == 1) {
                targetID = collapsed[1][targetID];
            } else {
                targetID++;
            }
            nextObj = svgdoc.getElementById("MI"+targetID);
            if (nextObj == null)
                break;
        } else {
            break;
        }
    }
    //save next uncollapsed element in array
    collapsed[1][clickedID] = targetID;
    if (nextObj == null)
        return;
    //move following texts upwards
    while (nextObj != null) {
        attr = nextObj.attributes;
        for (i=0;i<attr.length;i++) {
            if (attr.item(i).nodeName == "transform") {
                k = attr.item(i).nodeValue.indexOf("(");
                l = attr.item(i).nodeValue.indexOf(",");
                m = attr.item(i).nodeValue.indexOf(")");
                nextX = attr.item(i).nodeValue.substring(k+1,l);
                nextY = attr.item(i).nodeValue.substring(l+1,m);
            }
        }
        nextY -= gapY;
        nextObj.setAttribute("transform","translate("+nextX+","+nextY+")");
        targetID++;
        nextObj = svgdoc.getElementById("MI"+targetID);
    }
}

function init(evt)
{
    var nodenames, startnode, endnode, attr, i, j, k, l, m, nodesx, nodeex;
    collapsed[0] = new Array(%i);
    collapsed[1] = new Array(%i);
    for (i=0; i<%i; i++) {
        collapsed[0][i] = 0;
        collapsed[1][i] = 0;
    }
    for (i=0; i<%i; i++) {
	name[i] = "";
	clickStatus[i] = 0;
	redCount[i] = 0;
	salmonCount[i] = 0;
    }
    getSVGDoc(evt);

    //check which edges are printed from right to left
    paths = svgdoc.getElementsByTagName("path");
    revert = new Array(paths.length);
    for (j=0; j<paths.length; j++) {
	nodenames = paths.item(j).getAttribute("id").split("-");
	startnode = svgdoc.getElementById(nodenames[0]);
	attr = startnode.parentNode.attributes;
	for (i=0;i<attr.length;i++) {
	    if (attr.item(i).nodeName == "transform") {
		k = attr.item(i).nodeValue.indexOf("(");
		l = attr.item(i).nodeValue.indexOf(",");
		m = attr.item(i).nodeValue.indexOf(")");
		nodesx = parseFloat(attr.item(i).nodeValue.substring(k+1,l));
	    }
	}
	endnode = svgdoc.getElementById(nodenames[1]);
	attr = endnode.parentNode.attributes;
	for (i=0;i<attr.length;i++) {
	    if (attr.item(i).nodeName == "transform") {
		k = attr.item(i).nodeValue.indexOf("(");
		l = attr.item(i).nodeValue.indexOf(",");
		m = attr.item(i).nodeValue.indexOf(")");
		nodeex = parseFloat(attr.item(i).nodeValue.substring(k+1,l));
	    }
	}
	if (nodesx > nodeex) {
	    revert[j] = 1;
	} else {
	    revert[j] = 0;
	}
    }
}

function setStatus(evt, color1, color2)
{
    var clickObj = evt.getTarget();
    var clickObjName = clickObj.getAttribute('id');

    //find i corresponding to the clicked object
    for (i=0; i<%i; i++) {
	if (name[i] == "") {
	    name[i] = clickObjName;
	    break;
	}
	if (name[i] != clickObjName)
	    continue;
	break;
    }

    //toggle click status, color clicked object
    if (clickStatus[i] == 0) {
	clickStatus[i] = 1;
	clickObj.setAttribute("style","fill: "+color1);
    } else {
	clickStatus[i] = 0;
	clickObj.setAttribute("style","fill: "+color2);
    }

    //adjust color-counter
    if (color1 == 'red') {
	if (clickStatus[i] == 1) {
	    redCount[i]++;
	} else {
	    redCount[i]--;
	}
    }
    if (color1 == 'salmon') {
	if (clickStatus[i] == 1) {
	    salmonCount[i]++;
	} else {
	    salmonCount[i]--;
	}
    }

    if (clickStatus[i] == 0 && salmonCount[i] > 0) {
	clickObj.setAttribute("style","fill: salmon");
    }
}

function changeColor(evt, targetObjName, color1, color2)
{
    var clickObj = evt.getTarget();
    var clickObjName = clickObj.getAttribute('id');
    var targetObj = svgDocument.getElementById(targetObjName);

    //find i corresponding to the clicked object
    for (i=0; i<%i; i++) {
	if (name[i] != clickObjName)
	    continue;
	break;
    }

    //find j corresponding to the target object
    for (j=0; j<%i; j++) {
	if (name[j] == "") {
	    name[j] = targetObjName;
	    break;
	}
	if (name[j] != targetObjName)
	    continue;
	break;
    }

    //adjust color-counter
    if (color1 == 'red') {
	if (clickStatus[i] == 1) {
	    redCount[j]++;
	} else {
	    redCount[j]--;
	}
    }
    if (color1 == 'salmon') {
	if (clickStatus[i] == 1) {
	    salmonCount[j]++;
	} else {
	    salmonCount[j]--;
	}
    }
}

function colorText(targetObjName, color)
{
    var targetObj = svgDocument.getElementById(targetObjName);

    //find i corresponding to the target object
    for (i=0; i<%i; i++) {
	if (name[i] != targetObjName)
	    continue;
	break;
    }

    //color text
    if (i == %i) {
	targetObj.setAttribute("style","fill: "+color);
	return;
    }
    if (redCount[i] == 0 && salmonCount[i] == 0) {
	targetObj.setAttribute("style","fill: "+color);
    }
    if (salmonCount[i] > 0) {
	if (color == 'red') {
	    targetObj.setAttribute("style","fill: red");
	} else {
	    if (redCount[i] > 0) {
		targetObj.setAttribute("style","fill: red");
	    } else {
		targetObj.setAttribute("style","fill: salmon");
	    }
	}
    }
}
